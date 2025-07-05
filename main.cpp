#include <pspkernel.h>
#include <pspjpeg.h> 
#include <psputility_modules.h>

#define ATTR_PSP_WIDTH 480
#define ATTR_PSP_HEIGHT 272

PSP_MODULE_INFO("Application", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_MAIN_THREAD_STACK_SIZE_KB(1024*2);
PSP_HEAP_SIZE_KB(-1024*4);

#include <raylib.h>
#include <math.h>
#include <cstdio>
#include <ctype.h>
#include <string.h>

void getJpegSize(uint8_t * buf, int image_data_size, int &w, int &h) {
    for (int i = 2; i < image_data_size;) {
        if (buf[i] == 0xFF) {
            i++;
            switch(buf[i]) {
                case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC5: case 0xC6: case 0xC7: case 0xC9: case 0xCA: case 0xCB: case 0xCD: case 0xCE: case 0xCF:
                    i += 4;
                    h = (buf[i] << 8) | (buf[i+1]);
                    w = (buf[i+2] << 8) | (buf[i+3]);
                    i = image_data_size; break;
                case 0xDA: case 0xD9: break;
                default:
                    i += ((buf[i+1] << 8) | (buf[i+2])) + 1;
                break;
            }
        } else i++;
    }
}

unsigned char* resize_image_to_x_y(uint8_t * dst_data, const unsigned char* src_data, int src_width, int src_height, int bytes_per_pixel, int dst_width, int dst_height) {
    float x_ratio = (float) src_width / dst_width;
    float y_ratio = (float) src_height / dst_height;

    for (int y = 0; y < dst_height; y++) {
        for (int x = 0; x < dst_width; x++) {
            int src_x = (int) (x * x_ratio);
            int src_y = (int) (y * y_ratio);
            int dst_index = (y * dst_width + x) * bytes_per_pixel;
            int src_index = (src_y * src_width + src_x) * bytes_per_pixel;
            memcpy(&dst_data[dst_index], &src_data[src_index], bytes_per_pixel);
        }
    }

    return dst_data;
}

//my resizing function is a magic spell that turns images invisible, but jpegs don't have an alpha channel anyway
void max_alpha(unsigned char* data, const int width, const int height) {
    for (long i = 0; i < width * height * 4; i+=4) data[i+3] = 255;
}

#define DEBUG_LOG_SIZE 24
static char CustomLogBuffer[DEBUG_LOG_SIZE][128] = { 0 };
void CustomLog(int msgType, const char *text, va_list args) {
    char* customLogLine = CustomLogBuffer[0];
    for (int i = DEBUG_LOG_SIZE-1; i > 0 ; i--) strcpy(CustomLogBuffer[i], CustomLogBuffer[i-1]);
    switch (msgType) {
        case LOG_INFO: sprintf(customLogLine,    "[INFO] : "); break;
        case LOG_ERROR: sprintf(customLogLine,   "[ERROR]: "); break;
        case LOG_WARNING: sprintf(customLogLine, "[WARN] : "); break;
        case LOG_DEBUG: sprintf(customLogLine,   "[DEBUG]: "); break;
        default: break;
    }
    const int cur = strlen(customLogLine);
    vsnprintf(customLogLine+cur, 127-cur, text, args);
    sceIoWrite(1, customLogLine, strlen(customLogLine));
    sceIoWrite(1, "\n", 1);
}

void DrawLogData() {
    for (int i = 0; i < DEBUG_LOG_SIZE; i++) DrawText(CustomLogBuffer[i], 10, i * 10 + 30, 10, GREEN);
}

Texture loadJpegTextureME(uint8_t * buf, const uint32_t bufSize) {
    int decRes, w, h;
    getJpegSize(buf, bufSize, w, h);
    sceJpegCreateMJpeg(w, h);
    uint8_t *conversion = (uint8_t *) malloc(w*h*4);
    decRes = sceJpegDecodeMJpeg(buf, bufSize, conversion, 0);
    sceJpegDeleteMJpeg();

    uint8_t resized[64*64*4] __attribute__((aligned(16))); //need 16 byte alignment, i used the stack
    max_alpha(resize_image_to_x_y(resized, conversion, w, h, 4, 64, 64), 64, 64); //the psp doesn't appreciate big 32bpp textures
    free(conversion);

    Image img;
    img.width = 64;
    img.height = 64;
    img.data = resized;
    img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
    img.mipmaps = 1;

    TraceLog(LOG_INFO, "%d %d %c %d %d %d", w, h, decRes < 0 ? 'n':'y', decRes >> 16, decRes & 0xffff, decRes);

    return LoadTextureFromImage(img);
}

int main() {
    sceUtilityLoadModule(PSP_MODULE_AV_AVCODEC);
    sceJpegInitMJpeg();

    constexpr int screenWidth = ATTR_PSP_WIDTH;
    constexpr int screenHeight = ATTR_PSP_HEIGHT;
    InitWindow(screenWidth, screenHeight, nullptr);
    SetTraceLogCallback(CustomLog);
    SetTargetFPS(60);

    FILE * testFile = fopen("file.jpg", "rb");
    fseek(testFile, 0, SEEK_END);
    const size_t fileSize = ftell(testFile);
    fseek(testFile, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *) malloc(fileSize);
    fread(buf, 1, fileSize, testFile);
    fclose(testFile);

    Texture tex = loadJpegTextureME(buf, fileSize);
    free(buf);

    while (true) {
        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexturePro(tex, {0, 0, 64, 64}, {0, 0, 256, 256}, {}, 0, WHITE);
        DrawLogData();
        EndDrawing();
    }

    UnloadTexture(tex);

    sceJpegFinishMJpeg();
    return 0;
}
