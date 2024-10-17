// intialise buggy using PlatformIO and a version of arduino
// such as bluetooth, SD card, motors etc

// receive a bitmap over bluetooth and serial
// write the received bitmap to an SD card

// program will pull 12 rows at a time (each row representing a nozzle)
// typically to print an entire A4 sheet there will be 948 rows in the bitmap
// by taking 12 rows at a time this will take up roughly 1.42kb of memory

// idea is that 0 = white, 1 = black, 2 = end of row
void main() {
    int bitmap[12][12] = {{0, 1, 1, 0, 0, 1, 2, 2, 2, 2, 2, 2}, 
                        {0, 1, 0, 0, 1, 2, 2, 2, 2, 2, 2, 2}, 
                        {1, 0, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2},
                        {0, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2},
                        {1, 0, 0, 0, 0, 1, 0, 1, 2, 2, 2, 2}, 
                        {1, 0, 1, 1, 0, 1, 1, 1, 2, 2, 2, 2}, 
                        {1, 0, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2},
                        {1, 1, 0, 0, 1, 1, 2, 2, 2, 2, 2, 2},
                        {1, 0, 0, 0, 1, 0, 1, 0, 1, 2, 2, 2}, 
                        {1, 1, 0, 0, 0, 1, 1, 2, 2, 2, 2, 2}, 
                        {1, 1, 1, 0, 0, 0, 1, 0, 1, 2, 2, 2},
                        {1, 1, 0, 0, 1, 1, 0, 1, 2, 2, 2, 2},
                        };

    
}

