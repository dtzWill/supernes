#Build various ipk's
.PHONY: cat_beta cat preware pkg update

all: cat_beta cat preware

#Make sure the binary is up-to-date
../binaries/armv7/drnoksnes: ../drnoksnes
	cp $< $@

update: ../binaries/armv7/drnoksnes

cat_beta:
	$(MAKE) pkg ID=org.webosinternals.supernes.beta

cat:
	$(MAKE) pkg ID=org.webosinternals.supernes

preware:
	$(MAKE) pkg ID=org.webosinternals.supernes.preware

#Build a pkg with the specified 'ID'
pkg: update
	@if [ -z $(ID) ]; then \
		echo "Don't run this target directly!"; \
		exit 1; \
	fi
	cp pkg_base pkg_build -r
	cp ../binaries/armv7/drnoksnes pkg_build/
	sed -i s/@APPID@/$(ID)/ pkg_build/appinfo.json
	palm-package --use-v1-format pkg_build
	rm -rf pkg_build
	mkdir -p ipk
	mv *.ipk ipk

clean:
	-rm -rf pkg_build
	-rm -rf ipk
