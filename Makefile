d-build:
	docker build ./ -t kos:1.1.1.40u20.04

develop:
	docker run --net=host --volume="`pwd`:/data" --name demo-e2c -w /data --user user -it --rm kos:1.1.1.40u20.04 bash

build-sim:
	./cross-build.sh

sim:
	./cross-build-sim.sh

get-json-parser:
	git clone https://github.com/json-parser/json-parser

prepare: get-json-parser

clean:
	rm -rf build
