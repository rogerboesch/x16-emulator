//
//  Platform.m
//  Platform specfic includes for iOS (non SDL), see platform_ios.h
//
//  Written 2021 by Roger Boesch
//  "You can do whatever you like with it"
//

#import <UIKit/UIKit.h>
#include <sys/time.h>

char* platform_rom_file(void) {
    NSString* path = [[NSBundle mainBundle] pathForResource:@"rom" ofType:@"bin"];
    return (char *)[path UTF8String];
}

char* platform_get_base_path(void) {
    NSString* path = [[NSBundle mainBundle] resourcePath];
    path = [NSString stringWithFormat:@"%@/", path];
    return (char *)[path UTF8String];
}

char* platform_get_documents_path(void) {
    NSArray* paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,NSUserDomainMask, YES);
    NSString* documentsDirectory = [paths objectAtIndex:0];
    return (char *)[documentsDirectory UTF8String];
}

char* platform_get_gif_path(void) {
    NSString* path = [[NSURL fileURLWithPath:NSHomeDirectory()] path];
    path = [NSString stringWithFormat:@"%@/record.gif", path];
    
    NSLog(@"GIF path is: %@", path);
    
    return (char *)[path UTF8String];
}

double platform_get_ticks(void) {
    CFAbsoluteTime timeInSeconds = CFAbsoluteTimeGetCurrent();
    return timeInSeconds*1000.0;
}

FILE* platform_file_open(char* path, char* mode) {
    if (path[0] != '/') {
        // Load/save is just possible in document directory
        char fullPath[256];
        sprintf(fullPath, "%s/%s", platform_get_documents_path(), path);
    
        return fopen(fullPath, mode);
    }
    
    return fopen(path, mode);
}

void platform_file_close(FILE* fp) {
    fclose(fp);
}

size_t platform_file_read(FILE* fp, void* buf, size_t size, size_t mnemb) {
     return fread(buf, size, mnemb, fp);
}

size_t platform_file_write(FILE* fp, void* buf, size_t size, size_t mnemb) {
    return fwrite(buf, size, mnemb, fp);
}

uint8_t platform_file_read_u8(FILE* fp) {
    uint8_t buffer;
    fread(&buffer, 1, 1, fp);
    return buffer;
}

void platform_file_write_u8(FILE* fp, uint8_t byte) {
    fwrite (&byte, 1, 1, fp);
}

int platform_file_seek(FILE *fp, long int offset, int whence) {
    return fseek(fp, offset, whence);
}

char* platform_get_from_clipboard(void) {
    NSString* str = [UIPasteboard generalPasteboard].string;
    if (str == NULL) {
        return "";
    }
    
    return (char *)[str UTF8String];
}
