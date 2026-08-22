PROJECT_ROOT := $(CURDIR)

BUILD_DIR := $(PROJECT_ROOT)/build

export PROJECT_ROOT
export BUILD_DIR


COMPONENTS := $(shell \
	find . -mindepth 2 -name Makefile \
		-not -path "./build/*" \
		-not -path "./.git/*" \
		-exec grep -l "AP_COMPONENT := yes" {} \; \
	| xargs -n1 dirname \
	| sed 's#^\./##' \
	| awk '\
		/^core/      { core=core $$0 "\n" } \
		/^adapters\// { adapters=adapters $$0 "\n" } \
		/^plugins\//  { plugins=plugins $$0 "\n" } \
		/^apps\//     { apps=apps $$0 "\n" } \
		END { \
			printf "%s", core; \
			printf "%s", adapters; \
			printf "%s", plugins; \
			printf "%s", apps; \
		}' \
)
$(info COMPONENTS=$(COMPONENTS))

.PHONY: all clean $(COMPONENTS)


all: $(COMPONENTS)


$(COMPONENTS):
	$(MAKE) -C $@


clean:
	rm -rf $(BUILD_DIR)