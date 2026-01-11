# txt2svg

A high-performance text-to-SVG converter built with WebAssembly and stb_truetype. Converts text strings into SVG path data using TrueType fonts, perfect for generating dynamic text graphics on the web.

## Features

- **WebAssembly Performance**: Core rendering engine written in C and compiled to WASM for maximum speed
- **TrueType Font Support**: Uses stb_truetype library for accurate font rendering
- **Multiple Fonts**: Includes Roboto, Wendy One, and Alfa Slab One fonts
- **Custom Colors**: Support for any hex color
- **Cloudflare Workers Ready**: Deploy as a serverless API with built-in asset serving
- **Zero Dependencies**: Self-contained WASM module with no external runtime dependencies
- **SVG Output**: Generates clean, scalable SVG graphics

## Architecture

The project consists of several components:

### Core Components

- **`txt2svg.c`**: Main C implementation using stb_truetype for font rendering
- **`txt2svg.js`**: JavaScript wrapper for WASM module
- **`worker.js`**: Cloudflare Workers API server
- **`public/index.html`**: Demo interface and documentation

### Build System

- **`Makefile`**: Build targets for WASM compilation and local development
- **`wrangler.toml`**: Cloudflare Workers configuration

## Quick Start

### Prerequisites

- Clang (for WASM compilation)
- WebAssembly Binary Toolkit (wabt) for `wasm-opt`
- Node.js and npm (for Cloudflare Workers development)
- Cloudflare Workers account (for deployment)

### Installation

1. Clone the repository:
```bash
git clone https://github.com/thaolt/txt2svg.git
cd txt2svg
```

2. Build the WASM module:
```bash
make txt2svg.wasm
```

3. Start local development:
```bash
make dev
```

4. Open http://localhost:8787 to see the demo

## Usage

### Web API

The service provides a simple REST API:

```
GET /service?text=Hello%20World&color=ff0000&font=roboto
```

**Parameters:**
- `text`: The text to render (default: "Hello World")
- `color`: Hex color code without # (default: "000000")
- `font`: Font name - roboto, wendyone, alfaslabone (default: "roboto")

**Response:** SVG content with `Content-Type: image/svg+xml`

### Example Usage

#### HTML Image Tag
```html
<img src="https://your-worker-url/service?text=Hello&color=ff0000&font=roboto" alt="SVG Text" />
```

#### CSS Background
```css
.header {
  background-image: url("https://your-worker-url/service?text=Header&color=333333&font=alfaslabone");
}
```

#### JavaScript Fetch
```javascript
fetch('/service?text=Dynamic Text&color=0066cc&font=wendyone')
  .then(response => response.text())
  .then(svg => {
    document.getElementById('container').innerHTML = svg;
  });
```

### Local Development

For local testing without Cloudflare Workers:

```bash
# Build local executable
make local

# Run with default font
./local
```

## Available Fonts

- **Roboto**: Clean, modern sans-serif (468KB)
- **Wendy One**: Playful display font (21KB)
- **Alfa Slab One**: Bold slab serif (93KB)

## Build Targets

### Makefile Commands

```bash
# Build WASM module
make txt2svg.wasm

# Generate WebAssembly text format (for debugging)
make txt2svg.wat

# Start Cloudflare Workers development server
make dev

# Build local test executable
make local

# Clean build artifacts
make clean
```

### WASM Compilation

The WASM module is compiled with aggressive optimizations:
- Target: `wasm32-unknown-unknown`
- Memory: 8MB initial/maximum
- Optimization: `-Os` with LTO
- Binary size optimization with `wasm-opt`

## Technical Details

### Font Rendering Process

1. **Font Loading**: TrueType fonts are loaded into WASM memory
2. **Text Processing**: Each character is converted to glyph indices
3. **Path Generation**: Glyph outlines are converted to SVG path commands
4. **Layout**: Characters are positioned with proper kerning and spacing
5. **SVG Assembly**: Complete SVG document with proper dimensions

### Memory Management

- Custom WASM heap allocator for font data and temporary buffers
- 64KB output buffer for generated SVG
- Automatic memory cleanup between requests

### Mathematical Functions

The implementation includes custom math functions to avoid external dependencies:
- Square root (Newton's method)
- Logarithm and exponential (Taylor series)
- Trigonometric functions (polynomial approximation)

## Deployment

### Cloudflare Workers

1. Install Wrangler CLI:
```bash
npm install -g wrangler
```

2. Authenticate with Cloudflare:
```bash
wrangler login
```

3. Deploy:
```bash
wrangler deploy
```

### Custom Domains

Configure custom domains through the Cloudflare dashboard or Wrangler CLI for production use.

## Performance

- **Cold Start**: ~50ms for WASM initialization
- **SVG Generation**: ~5-20ms depending on text length and complexity
- **Memory Usage**: ~8MB fixed allocation
- **File Sizes**: WASM module ~15KB after optimization

## API Response Headers

```
Content-Type: image/svg+xml
Access-Control-Allow-Origin: *
Cache-Control: public, max-age=31536000
```

## Error Handling

The API returns appropriate HTTP status codes:

- `200`: Success
- `400`: Unsupported font or invalid parameters
- `500`: Internal server error (font loading, WASM errors)

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test locally with `make dev`
5. Submit a pull request

### Adding New Fonts

1. Add TTF file to `public/fonts/`
2. Update font mapping in `worker.js`
3. Update HTML demo options
4. Test the new font

## License

This project uses:
- stb_truetype (public domain)
- Custom C implementation (MIT)
- JavaScript wrapper (MIT)

## Troubleshooting

### Common Issues

**WASM compilation fails:**
- Ensure Clang supports WASM target
- Install wabt for `wasm-opt`

**Font loading errors:**
- Check font file permissions
- Verify font files are in `public/fonts/`

**Cloudflare Workers deployment:**
- Ensure `wrangler.toml` is configured correctly
- Check asset binding configuration

### Debug Mode

Generate WebAssembly text format for debugging:
```bash
make txt2svg.wat
# View the generated WebAssembly in human-readable format
```

## Roadmap

- [ ] Support for additional font formats (WOFF, WOFF2)
- [ ] Text alignment options (center, right)
- [ ] Multi-line text support
- [ ] Font size customization
- [ ] Text shadows and effects
- [ ] SVG optimization options
