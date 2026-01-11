export default function (wasm_module) {
    const { memory, wasm_generate_svg, wasm_alloc, wasm_reset_heap } = wasm_module.exports;
    const mem = new Uint8Array(memory.buffer);

    function writeStr(str) {
        const encoder = new TextEncoder();
        const buf = encoder.encode(str + "\0");
        const p = wasm_alloc(buf.length);
        mem.set(buf, p);
        return p;
    }
    
    function writeBin(buf) {
        const p = wasm_alloc(buf.length);
        mem.set(buf, p);
        return p;
    }
    
    this.generateSVG = (text, fontBuf, color) => {
        // reset heap inside WASM so that stb_truetype allocations and
        // our own buffers share a consistent allocator
        wasm_reset_heap();

        // if color does not start with '#' prepend it
        if (!color.startsWith('#')) {
            color = '#' + color;
        }
        
        const textPtr = writeStr(text);
        const colorPtr = writeStr(color);
        const fontPtr = writeBin(fontBuf);
        const outPtr = wasm_alloc(64 * 1024);
        
        const resultLength = wasm_generate_svg(
            textPtr,
            fontPtr,
            fontBuf.length,
            colorPtr,
            outPtr,
            64 * 1024
        );
        
        const decoder = new TextDecoder();
        const sliceStart = outPtr;
        const sliceEnd = mem.indexOf(0, outPtr);
        
        let result;
        if (sliceEnd === -1) {
            if (resultLength > 0) {
                result = decoder.decode(mem.slice(sliceStart, sliceStart + resultLength));
            } else {
                result = '';
            }
        } else {
            result = decoder.decode(mem.slice(sliceStart, sliceEnd));
        }
        
        return result;
    }
}