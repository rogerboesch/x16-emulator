//
//  Platform.m
//  x16-ios
//
//  Created by Roger Boesch on 18.02.21.
//

#import <Foundation/Foundation.h>

char* platform_rom_file(void) {
    NSString* path = [[NSBundle mainBundle] pathForResource:@"rom" ofType:@"bin"];
    return (char *)[path UTF8String];
}

char* platform_get_base_path(void) {
    NSString* path = [[NSBundle mainBundle] resourcePath];
    path = [NSString stringWithFormat:@"%@/", path];
    return (char *)[path UTF8String];
}

char* platform_get_gif_path(void) {
    NSString* path = [[NSURL fileURLWithPath:NSHomeDirectory()] path];
    path = [NSString stringWithFormat:@"%@/record.gif", path];
    
    NSLog(@"GIF path is: %@", path);
    
    return (char *)[path UTF8String];
}

int platform_get_ticks(void) {
    return 0;
}

FILE* platform_file_open(char* path, char* mode) {
    return fopen(path, mode);
}

size_t platform_file_read(FILE* fp, void* buf, size_t size, size_t mnemb) {
    return fread(buf, size, mnemb, fp);
}
