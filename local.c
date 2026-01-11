#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

extern unsigned int wasm_generate_svg(void* textPtr, void* fontPtr, unsigned int fontLen,
    void* colorPtr, void* outPtr, unsigned int outSize);

int main() {
    FILE *font_file = fopen("public/fonts/Roboto.ttf", "rb");
    if (!font_file) {
        printf("Failed to open font file\n");
        return 1;
    }
    fseek(font_file, 0, SEEK_END);
    long font_size = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);
    char *font = (char*)malloc(font_size);
    fread(font, 1, font_size, font_file);
    fclose(font_file);
    
    char text[] = "Hello World";
    char bg[] = "#000000";
    char color[] = "#ffffff";
    char out[32*1024];
    wasm_generate_svg((void*)text, (void*)font, font_size, (void*)color, (void*)out, sizeof(out));
    printf("%s\n", out);
    free(font);
    return 0;
}
