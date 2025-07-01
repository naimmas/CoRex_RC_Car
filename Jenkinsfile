def targets = ['stm32', 'rp2040']
pipeline {
    agent any

    options {
        timestamps()
        skipDefaultCheckout()
        buildDiscarder(logRotator(numToKeepStr: "10"))
        timeout(time: 30, unit: "MINUTES")
    }

    environment {
        BUILD_OUT = "build"
        BUILD_TYPE = "Debug"
    }

    stages {
        stage("Checkout") {
            steps {
                cleanWs()
                checkout scm
                sh "make clean-all"
            }
        }
        
        stage("Build") {
            steps {
                script {
                    for (TARGET in targets) {
                        echo "==== Building for ${TARGET} ===="
                        sh "make cont-build MCU=${TARGET} BUILD_TYPE=${BUILD_TYPE}"
                        recordIssues(
                            name: "build ${TARGET}-${BUILD_TYPE}",
                            enabledForFailure: true, 
                            tools: [gcc(pattern: "build/**/build_${TARGET}_${BUILD_TYPE}.log")],
                            id: "gcc-${TARGET}-${BUILD_TYPE}"
                        )
                    }
                }
            }
        }

        stage("Unit Tests") {
            steps {
                script {
                    for (TARGET in targets) {
                        sh "make cont-cov MCU=${TARGET}"
                        sh "cp build/artifacts/gcov/junit_tests_report.xml build/source/${TARGET}/junit_tests_report.xml"
                        sh "cp build/artifacts/gcov/gcovr/GcovCoverage.json build/source/${TARGET}/GcovCoverage.json"
                    }
                }
            }
        }

        stage("Static Analysis") {
            steps {
                script {
                    for (TARGET in targets) {
                        echo "==== Static Analysis for ${TARGET} ===="
                        sh "make cont-analyse-all MCU=${TARGET} BUILD_TYPE=${BUILD_TYPE}"
                    }
                }
            }
        }

        stage("Flaws Analysis") {
            steps {
                script {
                    echo "==== Security Analysis ===="
                    sh "make cont-analyse-flaws"
                    recordIssues(
                        tools: [flawfinder(
                            id      : 'flawfinder',
                            pattern : 'build/logs/ff_report.log',
                            )],
                        )
                    publishHTML (
                        target : [allowMissing: false,
                        alwaysLinkToLastBuild: true,
                        keepAll: true,
                        reportDir: 'build/logs',
                        reportFiles: 'liz_report.html',
                        reportName: 'Lizard Report'])
                    recordIssues(
                            name: "Lizard",
                            enabledForFailure: true, 
                            tools: [gcc(
                                pattern: "build/logs/liz_warns.log")]
                    )
                }
            }
        }
    }

    post {
        success {
            sh "make cont-exec CMD=\"gcovr -a build/source/stm32/GcovCoverage.json -a build/source/rp2040/GcovCoverage.json --cobertura build/logs/TestCoverageReport.xml\""
            sh "make cont-exec CMD=\"junitparser merge build/source/stm32/junit_tests_report.xml build/source/rp2040/junit_tests_report.xml build/logs/junit_tests_report.xml\""
            sh "awk \' \
                    /^\\// { \
                    if (block && !seen[block]++) print block; \
                    block = \$0 ORS; \
                    next; \
                    } \
                    { \
                    block = block \$0 ORS; \
                    } \
                    END { \
                    if (block && !seen[block]++) print block; \
                    } \
                    \' build/source/rp2040/Debug/clang-tidy.txt build/source/stm32/Debug/clang-tidy.txt > build/logs/clang-tidy.log"

            recordIssues(
                name: "Static Analyse",
                enabledForFailure: true,
                tool: clangTidy(pattern: "build/logs/clang-tidy.log")
            )            
            junit (
                testResults: "**/build/logs/junit_tests_report.xml",
                allowEmptyResults: true,
                keepTestNames: true,
            )
            recordCoverage(
                tools: [[parser: "COBERTURA",
                    pattern: "**/build/logs/TestCoverageReport.xml"]]
            )
        }
    }
}