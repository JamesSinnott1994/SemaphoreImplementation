#include "stdafx.h"
#include <iostream>
#include <list>
#include <SDL.h>
#include <fstream>
#include <string>

//Data access semaphore
SDL_sem* rw = NULL;// Database
SDL_sem* readerMutex = NULL;

int Reader(void * data);
int Writer(void * data);

int nr = 0; // Number of readers and writers

int _tmain(int argc, _TCHAR* argv[])
{
	std::list<SDL_Thread*> readers;
	std::list<SDL_Thread*> writers;

	//Initialize semaphore
	rw = SDL_CreateSemaphore(1);
	readerMutex = SDL_CreateSemaphore(1);

	// Create multiple reader and writer threads
	for (int i = 0; i < 5; i++)
	{
		readers.push_back(SDL_CreateThread(Reader, "reader", NULL));
		writers.push_back(SDL_CreateThread(Writer, "writer", &i));
	}

	std::cin.get();
	return 0;
}

int Reader(void *data)
{
	while (true)
	{
		// ENTRY SECTION
		SDL_SemWait(readerMutex);// Lock access to nr
		nr += 1;// Indicate you are a reader trying to enter the CS
		if (nr == 1)//Checks if you are the first reader trying to enter CS
		{
			SDL_SemWait(rw);// Lock the database 'rw' from writers
		}
		SDL_SemPost(readerMutex);// Unlock access to nr
		// ENTRY SECTION

		// CRITICAL SECTION
		std::cout << "Reading" << std::endl;

		std::ifstream myfile("Textfile.txt");// Open textfile
		std::string line;

		if (myfile.is_open())
		{
			while (getline(myfile, line))
			{
				std::cout << line << '\n';// Output text
			}
			myfile.close();
		}
		// CRITICAL SECTION

		// EXIT SECTION
		SDL_SemWait(readerMutex);// Lock access to nr
		nr -= 1;// Indicate you are no longer reading the resource
		if (nr == 0)// Check if you are reader who is reading the shared file
		{
			SDL_SemPost(rw);// If you are the last reader, then unlock access to the resource so that it is available to writers.
		}
		SDL_SemPost(readerMutex);// Unlock access to nr
		// EXIT SECTION

		// Delay to give writer a chance to acquire a lock on the shared resource
		SDL_Delay(16 + rand() % 640);
	}
	return 0;
}

int Writer(void *data)
{
	int i = *(int *)data;
	while (true)
	{
		SDL_SemWait(rw);// Lock database from readers
		std::cout << "Writing" << std::endl;

		// Write to text file
		std::ofstream myfile;
		myfile.open("Textfile.txt");
		myfile << "Achievement: " << i << std::endl;
		myfile.close();

		SDL_SemPost(rw);// Unlock database from readers

		// Delay to give reader a chance to acquire a lock on the shared resource
		SDL_Delay(16 + rand() % 640);
	}
	return 0;
}