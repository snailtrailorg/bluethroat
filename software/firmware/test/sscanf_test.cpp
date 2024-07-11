#include <stdio.h>
#include <stdint.h>
#include <string.h>

int splite_sentence(char *sentence, char *fields[], int max_fields);

int main(void) {
	char sentence[256];
	strcpy(sentence, "$GNGGA,162026.00,2236.01250,N,11400.48146,E,1,08,4.59,161.8,M,-2.5,M,,*5F\r\n");

	uint32_t latitude[2];
	char latitude_direction = 'X';
	uint32_t longitude[2];
	char longitude_direction = 'X';
	float altitude;
	float undulation;
	char buffer[16];
	float second;

	/*$GNGGA,162026.00,2236.01250,N,11400.48146,E,1,08,4.59,161.8,M,-2.5,M,,*5F */
	if (sscanf(sentence, "$GNGGA,%*f,%d.%d,%c,%d.%d,%c,%*d,%*d,%*f,%f,M,%f,M", 
	&(latitude[0]), &(latitude[1]), &latitude_direction, 
	&(longitude[0]), &(longitude[1]), &longitude_direction, 
	&altitude, &undulation) == 8) {
		printf("latitude: %ld.%ld %c\n", latitude[0], latitude[1], latitude_direction);
		printf("longitude: %ld.%ld %c\n", longitude[0], longitude[1], longitude_direction);
		printf("altitude: %f\n", altitude);
		printf("undulation: %f\n", undulation);
		printf("altitude: %f\n", altitude);
	} else {
        	printf("Failed to parse\n");
	}

	char *fields[16];

	int count = splite_sentence(sentence, fields, 16);
	for (int i = 0; i < count; i ++) {
		printf("fields[%d]=%s\n", i, fields[i]);
	}

	char buffer1[16] = {0};
	char buffer2[16] = {0};

	strcpy(sentence, "hello,,world");
	
	int n = sscanf(sentence, "%[^,],%*[^,],%s", buffer1, buffer2);
	printf("n=%d, word1=%s, word2=%s\n", n, buffer1, buffer2);

	return 0;
}

int splite_sentence(char *sentence, char *fields[], int max_fields) {
    int count = 0;
    char *p = sentence;
    char *q = sentence;

    while (*p != '\0' || *p != '\n' || *p != '\r') {
        if (*p == ',' || *p == '*') {
            *p = '\0';
            fields[count] = q;
            count ++;
            q = p + 1;
        }

        if (count >= max_fields) {
            break;
        }

        p ++;
    }

    return count;
}