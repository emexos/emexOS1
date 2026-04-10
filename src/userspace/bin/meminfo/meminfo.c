#include <stdio.h>
#include <string.h>


long extract_mem_kb(const char *line)
{
	long value = 0;
	int found = 0;

	for (int i = 0; line[i] != '\0'; i++)
	{
		char c = line[i];

		if (c >= '0' && c <= '9')
		{
			value = value * 10 + (c - '0');
			found = 1;
		} else {
			if (found){
				/* until now -> finish */
				break;
			}
		}
	}

	return value;
}

int main()
{
	char line[256];
	long memTotalKB = 0;

	FILE *file = fopen("/proc/meminfo", "r");
	if (!file)
	{
		perror("meminfo is unavailable");
		return 1;
	}

	while (fgets(line, sizeof(line), file))
	{
		if (strncmp(line, "MemTotal:", 9) == 0){
			memTotalKB = extract_mem_kb(line);
		}
	}

	fclose(file);

	if (memTotalKB <= 0)
	{
		fprintf(stderr, "MemTotal could not be found\n");
		return 1;
	}

	long memMB = memTotalKB / 1024;
	long memGB = memMB / 1024;

	printf("MemTotal: %ld kB \n",  memTotalKB);
	printf("MemTotal: %ld MB (rounded to: %ld GB) \n", memMB, memGB);

	return 0;
}