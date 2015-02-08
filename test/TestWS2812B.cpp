
// STL includes
#include <vector>
#include <ctime>
#include <cstring>
#include <string>
#include <unistd.h>
#include <iostream>
#include <vector>

// Local includes
#include <utils/ColorRgb.h>

#include "../libsrc/leddevice/LedDeviceWS2812b.h"

bool _running = true;

#include <csignal>

void signal_handler(int signum)
{
	_running = false;
}


void setPWMBit(std::vector<uint32_t> &PWMWaveform, unsigned int bitPos, bool bit) {

	// Fetch word the bit is in
	unsigned int wordOffset = (int)(bitPos / 32);
	unsigned int bitIdx = bitPos - (wordOffset * 32);

	if (bit){
			PWMWaveform[wordOffset] |= (1 << (31 - bitIdx));
	} else {
			PWMWaveform[wordOffset] &= ~(1 << (31 - bitIdx));
	}
}

void setPWMColorBits(uint8_t color, std::vector<uint32_t> &PWMWaveform, unsigned int &wireBitPos){
	for (int i = 0; i < 8; i++){
		setPWMBit(PWMWaveform, wireBitPos++, 1);
		setPWMBit(PWMWaveform, wireBitPos++, (color && (0x8000 >> i)) > 0);
		setPWMBit(PWMWaveform, wireBitPos++, 0);
	}
}

// Convenience functions
// --------------------------------------------------------------------------------------------------
// Print some bits of a binary number (2nd arg is how many bits)
void printBinary(unsigned int i, unsigned int bits) {
	int x;
	for(x=bits-1; x>=0; x--) {
		printf("%d", (i & (1 << x)) ? 1 : 0);
		if(x % 16 == 0 && x > 0) {
			printf(" ");
		} else if(x % 4 == 0 && x > 0) {
			printf(":");
		}
	}
}

void testColorVector(LedDeviceWS2812b * ledDevice, std::vector<ColorRgb> * buff, const char* runName){
	//printf("Running test: %s\n", runName);
	unsigned int ledCnt = buff->size();
	ledDevice->write(*buff);
	struct control_data_s *ctl = (struct control_data_s *) ledDevice->virtbase;

	std::vector<uint32_t> compareBuff(((ledCnt * 2.25) + 1));

	///100 100 100 100 100 100 100 100
	///110 110 110 110 110 110 110 110
	///100 100 100 100 100 100 100 100
	//int bytepos = 0;
	unsigned int wireBit = 0;
	for (unsigned int i = 0; i < buff->size(); i++){
		setPWMColorBits((*buff)[i].green, compareBuff, wireBit);
		setPWMColorBits((*buff)[i].red, compareBuff, wireBit);
		setPWMColorBits((*buff)[i].blue, compareBuff, wireBit);
	}

	for (unsigned int i = 0; i < compareBuff.size(); i++){
		if (compareBuff[i] != ctl->sample[i]){
			printf("%s: Error on position %d of %d, LEDs: %d - DeviceLeds: %d\n", runName, i, compareBuff.size() - 1, ledCnt, ledDevice->mLedCount);
			printf("Expected:\n");

			if (i > 0){
				printf("Pos: %d: ", i-1);
				printBinary(compareBuff[i-1], 32);
				printf("\n");
			}

			printf("Pos: %d: ", i);
			printBinary(compareBuff[i], 32);
			printf("\n");

			if (i < compareBuff.size() -1){
				printf("Pos: %d: ", i+1);
				printBinary(compareBuff[i +1], 32);
				printf("\n");
			}

			printf("Created:\n");

			if (i > 0){
				printf("Pos: %d: ", i-1);
				printBinary(ctl->sample[i-1], 32);
				printf("\n");
			}

			printf("Pos: %d: ", i);
			printBinary(ctl->sample[i], 32);
			printf("\n");

			if (i < compareBuff.size() -1){
				printf("Pos: %d: ", i+1);
				printBinary(ctl->sample[i +1], 32);
				printf("\n");
			}
			printf("\n");
		}
	}

//	if (ctl->sample[compareBuff.size()] != 0x0){
//		printf("Error on position %d (after last used)\n", compareBuff.size());
//		printf("Expected: ");
//		printBinary(0, 32);
//		printf("\nCreated:  ");
//		printBinary(ctl->sample[compareBuff.size()], 32);
//		printf("\n");
//	}


//		printf("Error on position %d\n", compareBuff.size() +2);
//		printf("Expected: ");
//		printBinary(0, 32);
//		printf("\nCreated:  ");
//		printBinary(ctl->sample[compareBuff.size() +2], 32);
//		printf("\n");
}

int main(int argc, char** argv)
{
	if (sizeof(ColorRgb) != 3)
	{
		std::cout << "sizeof(ColorRgb) = " << sizeof(ColorRgb) << std::endl;
		return -1;
	}

	// Install signal handlers to stop loops
	signal(SIGTERM, &signal_handler);
	signal(SIGINT,  &signal_handler);

	unsigned ledCnt = 20;
	std::vector<ColorRgb> * buff;

	printf("Stating tests\n");
	LedDeviceWS2812b * ledDevice = new LedDeviceWS2812b();

	buff = new std::vector<ColorRgb>(ledCnt, ColorRgb::RED);
	testColorVector(ledDevice, buff, "RED");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt + 5, ColorRgb::BLUE);
	testColorVector(ledDevice, buff, "BLUE");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt - 4, ColorRgb::GREEN);
	testColorVector(ledDevice, buff, "GREEN");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt + 1, ColorRgb::YELLOW);
	testColorVector(ledDevice, buff, "YELLOW");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt - 1, ColorRgb::WHITE);
	testColorVector(ledDevice, buff, "WHITE");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt + 1, ColorRgb::YELLOW);
	testColorVector(ledDevice, buff, "YELLOW2");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt, ColorRgb::GREEN);
	testColorVector(ledDevice, buff, "GREEN2");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt + 1, ColorRgb::BLUE);
	testColorVector(ledDevice, buff, "BLUE2");
	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt -1, ColorRgb::RED);
	testColorVector(ledDevice, buff, "RED3");
	delete buff;

	sleep(1);

//	buff = new std::vector<ColorRgb>(ledCnt -1, ColorRgb::BLACK);
//	for (unsigned int i = 0; i < buff->size(); i++){
//		buff->at(i) = ColorRgb::YELLOW;
//		testColorVector(ledDevice, buff, "Counting");
//	}
//	delete buff;

	sleep(1);

	buff = new std::vector<ColorRgb>(ledCnt +5, ColorRgb::BLACK);
	testColorVector(ledDevice, buff, "BLACK");
	delete buff;

	delete ledDevice;

	printf("Tests finished!\n");
	printf("All LEDs should be black!\n");
	return 0;
}


