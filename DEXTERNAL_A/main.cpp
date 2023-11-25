#include "wapi.h"
#include "strutil.h"



// i had to re-write the entire code because i had some stuff like the b16 function returning true if it wasn't b16... (i built my code around that instead of reversing it)
// and also there were random buffers that were not necessary
// anyway so yeah i re-wrote it so that you could actually read it

/// if you just want the message spoofing part i have a MessageSpoofer repo so go see that


#define keydown 0
#define keyup 2


#define KEYBIND VK_F10


using namespace std::chrono;

int main()
{
	FindDiscord();
	auto duration = high_resolution_clock::now();

	for (;;)
	{

		// should probably remove sleep because two keys have a higher chance to be the same, but it uses an entire thread constantly
		while (!GetAsyncKeyState(KEYBIND)) { Sleep(3); }

		// cpy highlighted text
		inject(VK_CONTROL, keydown);
		Sleep(10);
		inject('c' - 0x20, keydown);
		Sleep(10);
		inject('c' - 0x20, keyup);
		Sleep(10);
		inject(VK_CONTROL, keyup);

		std::string ed = ReadClipboard();

		bool isB16 = b16(ed);

		if (isB16 == false) // if ! b16 then we encrypt
		{
			size_t encryptmelen = ed.length();
			char* encryptme = new char[encryptmelen + 1];
			for (size_t i = 0; i < encryptmelen; i++) {
				encryptme[i] = ed[i]; } 
			encryptme[encryptmelen] = 0;


			size_t hexlen = ((ed.length()) * 2) + 1;
			char* hexadecimal = new char[hexlen + 1];
			for (size_t i = 0; i < hexlen; i++) {
				hexadecimal[i] = 0; }

			hexadecimal[hexlen] = 0;

			// generate a "random" key (it's just hashing the time elapsed...)
			char key[keylen + 1] = {};
			for (size_t i = 0; i < keylen; i++) 
			{
				auto finish = std::chrono::high_resolution_clock::now();
				std::chrono::duration<double> elapsed = finish - duration;
				char random = (char)(XHASH(((uint64_t)((double)elapsed.count() * 1000)) + i) % 0x100);
				key[i] = random;
			}

			key[keylen] = 0;

			size_t resultbufferlen = hexlen + (keylen * 2) + 1;
			char* resultbuffer = new char[resultbufferlen + 1];
			for (size_t i = 0; i < resultbufferlen + 1; i++) {
				resultbuffer[i] = 0; }
			
			char* keystring = new char[(keylen * 2) + 1];
			keystring[keylen * 2] = 0;

			CRYPT(encryptme, key, encryptmelen);
			aob2h(encryptme, hexadecimal, encryptmelen);
			aob2h(key, keystring, keylen);

			joiner(keystring, keylen * 2, hexadecimal, hexlen, resultbuffer);

			std::cout << "\nencrypted: \n";
			for (size_t i = 0; i < resultbufferlen; i++)
			{
				std::cout << resultbuffer[i];
			} std::cout << "\n";
			
			WriteClipboard((const char*)resultbuffer);


			// pastes (i love pasting) and presses enter for you!
            inject(VK_CONTROL, keydown);
            Sleep(10);
            inject('v' - 0x20, keydown);
            Sleep(10);
            inject('v' - 0x20, keyup);
            Sleep(10);
            inject(VK_CONTROL, keyup);
            Sleep(10);
            inject(VK_RETURN, keydown);
            Sleep(10);
            inject(VK_RETURN, keyup);
			

			delete[] keystring;
			delete[] resultbuffer;
			delete[] hexadecimal;
			delete[] encryptme;
		}
		else if (isB16 == true) // if b16 then we decrypt fam
		{
			size_t recbufferlen = ed.length();
			char* recbuffer = new char[ed.length() + 1];
			recbuffer[recbufferlen] = 0;
			for (size_t i = 0; i < recbufferlen; i++)
			{
				recbuffer[i] = ed[i];
			}

			size_t hexlen = GetHexLen(recbuffer, recbufferlen);
			char* hexadecimal = new char[hexlen + 1];
			hexadecimal[hexlen] = 0;

			char keybuffer[keylen * 2];
			char key[keylen];
			splitter(recbuffer, recbufferlen, keybuffer, hexadecimal);
			h2aob(keybuffer, key, keylen);

			char* resbuffer = new char[(hexlen / 2) + 1];
			h2aob(hexadecimal, resbuffer, hexlen / 2);
			CRYPT(resbuffer, key, hexlen / 2);

			std::cout << "\ndecrypted:\n";
			for (size_t i = 0; i < hexlen / 2; i++)
			{
				std::cout << resbuffer[i];
			} std::cout << '\n';


			resbuffer[hexlen / 2] = 0;
			std::string convbuffer = (const char*)resbuffer;
			ReplaceMsg(ed, convbuffer);

			delete[] hexadecimal;
			delete[] resbuffer;
			delete[] recbuffer;
		}

		while (GetAsyncKeyState(KEYBIND)) { Sleep(3); }
	}
}