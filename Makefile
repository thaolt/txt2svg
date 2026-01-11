txt2svg.wasm: txt2svg.c
	clang \
		--target=wasm32-unknown-unknown \
		-mbulk-memory \
		-nostdlib \
		-Wl,--no-entry \
		-Wl,--export-all \
		-Wl,--allow-undefined \
		-Wl,--initial-memory=8388608 \
		-Wl,--max-memory=8388608 \
		-Wl,--strip-all \
		-Wl,--lto-O3 \
		-Wl,-O3 \
		-Os \
		txt2svg.c \
		-o txt2svg.wasm
	wasm-opt --enable-bulk-memory-opt -Oz txt2svg.wasm -o txt2svg.wasm

txt2svg.wat: txt2svg.wasm
	wasm2wat txt2svg.wasm > txt2svg.wat

dev: txt2svg.js worker.js
	wrangler dev

local: txt2svg.c local.c
	clang -g -o local local.c txt2svg.c -lm

clean:
	rm -f txt2svg.wasm txt2svg.wat local
