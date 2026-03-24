#include </usr/include/postgresql/libpq-fe.h>
#define QUERYSIZE 4048

typedef struct queryText
{
	int number;
	char* line;
	char** values;
	int numberOfValues;
	struct queryText* previous;
	struct queryText* next;
} queryText;

typedef struct rawData
{
	char*** data;
	int rows;
	int cols;
	struct rawData* previous;
	struct rawData* next;
} rawData;

queryText* mallocTextQueries(int allocations, int numberOfValues);
void freeTextQueries(queryText* queryToFree);
rawData* mallocRawData(int rows, int cols);
void freeRawData(rawData* toFree);
rawData* appendRawData(rawData* mainData, rawData* toAppend);
int executeQueryOnDatabase(queryText* queries);
rawData* executeReadingOnDataBase(queryText* queries);
