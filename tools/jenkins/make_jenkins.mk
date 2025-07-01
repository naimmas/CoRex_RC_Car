JENKINS_SERVER ?= localhost
JENKINS_JOB ?= $(shell basename $(shell pwd))

THIS_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
JENKINS_USER = $(shell jq -r '.user' $(THIS_DIR)/.priv_jauth)
JENKINS_PASS = $(shell jq -r '.token' $(THIS_DIR)/.priv_jauth)
JENKINS_URL := http://$(JENKINS_SERVER):8080
NEED_NETWORK = $(shell $(CONTAINER) network inspect jenkins > /dev/null 2>&1 || echo .bridge-network)
NEED_DIND = $(shell $(CONTAINER) ps -a | grep jenkins-docker > /dev/null 2>&1 || echo .run-dind)
NEED_BUILD = $(shell $(CONTAINER) ps -a | grep myjenkins > /dev/null 2>&1 || echo jenkins-build)
JENKINS_IMG_NAME = myjenkins-blueocean:2.510
JENKINS_CONTAINER_NAME = jenkins-blueocean

.bridge-network:
	@echo "Creating jenkins network..."
	@$(CONTAINER) network create jenkins
	@echo "Jenkins network created successfully."

.run-dind:
	@$(CONTAINER) run --name jenkins-docker --rm --detach \
	--privileged --network jenkins --network-alias docker \
	--env DOCKER_TLS_CERTDIR=/certs \
	--volume jenkins-docker-certs:/certs/client \
	--volume jenkins_home:/var/jenkins_home \
	--publish 2376:2376 docker:dind --storage-driver overlay2

jenkins-build: $(NEED_NETWORK) $(NEED_DIND)
	@echo "Building Jenkins image..."
	docker build --file $(THIS_DIR)/jenkins.Dockerfile -t $(JENKINS_IMG_NAME) .
	@echo "Jenkins image built successfully."

jenkins-start: $(NEED_BUILD) $(NEED_DIND)
	@echo "Starting Jenkins container..."
	@if ($(CONTAINER) ps -a | grep $(JENKINS_CONTAINER_NAME)); then \
		$(CONTAINER) start $(JENKINS_CONTAINER_NAME); \
	else \
		$(CONTAINER) run --name $(JENKINS_CONTAINER_NAME) \
		--restart=on-failure --detach \
		--network jenkins --env DOCKER_HOST=tcp://docker:2376 \
		--env DOCKER_CERT_PATH=/certs/client --env DOCKER_TLS_VERIFY=1 \
		--publish 8080:8080 --publish 50000:50000 \
		--volume jenkins_home:/var/jenkins_home \
		--volume jenkins-docker-certs:/certs/client:ro \
		$(JENKINS_IMG_NAME); \
		sleep 5; \
		while [ "$$(curl -s -w "%{http_code}" $(JENKINS_URL)/login -o /dev/null)" != "200" ]; do \
			echo "Waiting for Jenkins to start on $(JENKINS_URL) ..."; \
			sleep 5; \
		done; \
		$(CONTAINER) exec $(JENKINS_CONTAINER_NAME) sh -c 'if [ -f /var/jenkins_home/secrets/initialAdminPassword ]; then echo "Jenkins initial admin password:"; cat /var/jenkins_home/secrets/initialAdminPassword; fi'; \
	fi

jenkins-stop:
	@echo "Stopping Jenkins container..."
	@$(CONTAINER) stop $(JENKINS_CONTAINER_NAME)
	@$(CONTAINER) stop jenkins-docker
	@echo "Jenkins container stopped successfully."

jenkins-clean:
	@echo "Cleaning Jenkins container..."
	-@{ \
	$(CONTAINER) stop $(JENKINS_CONTAINER_NAME); \
	$(CONTAINER) stop jenkins-docker; \
	$(CONTAINER) network rm jenkins; \
	$(CONTAINER) container rm -f jenkins-docker; \
	$(CONTAINER) rm -f $(JENKINS_CONTAINER_NAME); \
	$(CONTAINER) rmi -f $(JENKINS_IMG_NAME); \
	$(CONTAINER) volume rm -f jenkins_home; \
	}
	@echo "Jenkins container cleaned successfully."

jenkins-ws-clean:
	@echo "Cleaning Jenkins workspace..."
	@$(CONTAINER) exec -u 0 $(JENKINS_CONTAINER_NAME) bash -c 'rm -rf /var/jenkins_home/workspace/*'
	@echo "Jenkins workspace cleaned successfully."

jenkins-validate:
	@curl -X POST \
	--user "$(JENKINS_USER):$(JENKINS_PASS)" \
	-F "jenkinsfile=<Jenkinsfile" \
	"$(JENKINS_URL)/pipeline-model-converter/validate"

jenkins-trigger:
	@curl -X POST \
	--user "$(JENKINS_USER):$(JENKINS_PASS)" \
	"$(JENKINS_URL)/job/$(JENKINS_JOB)/build"