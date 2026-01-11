import txt2svg_wasm from "./txt2svg.wasm";
const txt2svg_mod = await WebAssembly.instantiate(txt2svg_wasm, {
    env: {
        memory: new WebAssembly.Memory({ initial: 128 }) // 8MB (128 * 64KB pages)
    }
});
import TXT2SVG from "./txt2svg.js";

async function loadFont(font, req, env) {
    const fontsURLMap = {
        roboto: 'Roboto.ttf',
        wendyone: 'WendyOne.ttf',
        alfaslabone: 'AlfaSlabOne.ttf',
    }

    if (typeof fontsURLMap[font] === 'undefined')
        return null;
    
    try {
        const fontResponse = await env.ASSETS.fetch(new Request(new URL('/fonts/'+fontsURLMap[font], req.url)));
        const fontBuffer = await fontResponse.arrayBuffer();
        return new Uint8Array(fontBuffer);
    } catch (error) {
        console.error('Failed to load fonts:', error);
        throw error;
    }
}

export default {
    async fetch(req, env, ctx) {
        const url = new URL(req.url);
        const path = url.pathname;

        // Handle GET /
        if (req.method === 'GET') {
            if (path === '/service') {
                
                // execute the module and return generated SVG
                const urlParams = url.searchParams;
                const text = urlParams.get('text') || 'Hello World';
                const fontKey = (urlParams.get('font') || 'roboto').toLowerCase();
                const color = urlParams.get('color') || '#000000';

                const fontBuffer = await loadFont(fontKey, req, env);

                try {
                    // Load font
                    if (!fontBuffer) {
                        return new Response(JSON.stringify({ error: `Unsupported font: ${fontKey}` }), {
                            status: 400,
                            headers: { 'Content-Type': 'application/json' }
                        });
                    }
                    
                    // Initialize txt2svg
                    const txt2svg = new TXT2SVG(txt2svg_mod);
                    
                    // Generate SVG
                    console.log('txt2svg debug:', {
                        text,
                        fontKey,
                        fontLength: fontBuffer.length,
                    });

                    const svg = txt2svg.generateSVG(text, fontBuffer, color);

                    console.log('txt2svg debug result length:', svg ? svg.length : 0);
                    
                    if (!svg || svg.length === 0) {
                        return new Response(JSON.stringify({ error: 'Generated SVG is empty' }), {
                            status: 500,
                            headers: { 'Content-Type': 'application/json' }
                        });
                    }
                    
                    return new Response(svg, {
                        headers: {
                            'Content-Type': 'image/svg+xml',
                            'Access-Control-Allow-Origin': '*',
                            'Cache-Control': 'public, max-age=31536000' // Cache for 1 year
                        }
                    });
                } catch (error) {
                    console.error('Error generating SVG:', error);
                    return new Response(JSON.stringify({ error: 'Internal server error' }), {
                        status: 500,
                        headers: { 'Content-Type': 'application/json' }
                    });
                }
            } else {
                return env.ASSETS.fetch(req);
            }
        }

    }
}