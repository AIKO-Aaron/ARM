#pragma once

#include <decompiler/Decompiler.h>

static byte* readFile(const char* path) {
	FILE *f;
	fopen_s(&f, path, "rb");
	if (!f) {
		printf("Could not open file: %s\n", path);
		return nullptr;
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	byte* data = (byte*)malloc(size + 1);

	fread(data, 1, size, f);
	data[size] = 0;

	return data;
}