d-build:
	docker build ./ -t kos:1.2.0.89u22.04

develop:
	docker run --net=host --volume="`pwd`:/data" --name traffic-light-kos-day1 --user user -w /data -it --rm kos:1.2.0.89u22.04 bash


build:
	./cross-build.sh

sim:
	./cross-build-sim.sh

clean:
	rm -rf build