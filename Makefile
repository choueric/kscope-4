TOPDIR=`pwd`
BUILD_DIR="$(TOPDIR)/_build"

.PHONY: config, build, install, clean, help, run
.DEFAULT_GOAL := help

config: ## run cmake in _build
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake ..

build: ## run make
	@cd $(BUILD_DIR) && make

install: ## install 
	@cd $(BUILD_DIR) && sudo make install

run:  ## execute the kscope program
	@$(BUILD_DIR)/bin/kscope

clean: ## clean _build
	@rm -rf $(BUILD_DIR)

help:
	@grep -E '^[a-zA-Z0-9_-]+:.*?## .*$$' $(MAKEFILE_LIST) | awk 'BEGIN {FS = ":.*?## "}; {printf "\033[36m%-10s\033[0m %s\n", $$1, $$2}'
