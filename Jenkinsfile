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
                    sh "make cont-build BUILD_TYPE=${BUILD_TYPE}"
                    recordIssues(
                        name: "build ${BUILD_TYPE}",
                        enabledForFailure: true, 
                        tools: [gcc(pattern: "build/**/build_${BUILD_TYPE}.log")],
                        id: "gcc-${BUILD_TYPE}"
                    )
                }
            }
        }

        stage("Unit Tests") {
            steps {
                script {
                    sh "make cont-cov"
                    sh "cp build/artifacts/gcov/junit_tests_report.xml build/logs/junit_tests_report.xml"
                    sh "cp build/artifacts/gcov/gcovr/GcovCoverage.json build/logs/GcovCoverage.json"
                }
            }
        }

        stage("Static Analysis") {
            steps {
                script {
                    sh "make cont-analyse-all BUILD_TYPE=${BUILD_TYPE}"
                }
            }
        }

        stage("Flaws Analysis") {
            steps {
                script {
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
            recordIssues(
                name: "Static Analyse",
                enabledForFailure: true,
                tool: clangTidy(pattern: "build/source/${BUILD_TYPE}/clang-tidy.log")
            )
            junit (
                testResults: "build/logs/junit_tests_report.xml",
                allowEmptyResults: true,
                keepTestNames: true,
            )
            recordCoverage(
                tools: [[parser: "COBERTURA",
                    pattern: "build/logs/TestCoverageReport.xml"]]
            )
        }
    }
}