#include "../../include/database/database.h"
#include "../../include/globalStructures/globalStructures.h"

/*Allocation of the textQueries structures*/
queryText* mallocTextQueries(int allocations, int numberOfValues)
{
	queryText* queryToReturn = NULL;
	queryText* mainQuery = NULL;
	if(allocations <= 0)
		allocations = 1;

	for(int i = 0;i < allocations; i++)
	{
		queryText* toLoop = (queryText*)malloc(sizeof(queryText));
		toLoop->line = (char*)malloc(QUERYSIZE * sizeof(char));
		toLoop->values = (char**)malloc(numberOfValues * sizeof(char*));
		for(int j = 0; j < numberOfValues; j++)
			toLoop->values[j] = (char*)malloc(VALUESSIZE * sizeof(char*));
		toLoop->numberOfValues = numberOfValues;
		toLoop->previous = NULL;
		toLoop->next = NULL;

		if(mainQuery == NULL)
		{
			mainQuery = toLoop;
			queryToReturn = toLoop;
		} else
		{
			for(mainQuery; mainQuery->next != NULL; mainQuery = mainQuery->next);
			mainQuery->next = toLoop;
			toLoop->previous = mainQuery;
		}
	}

	return queryToReturn;
}

/*Frees the textQueries structures*/
void freeTextQueries(queryText* queryToFree)
{
	queryText* toLoop = queryToFree;
	for(toLoop; toLoop != NULL; toLoop = queryToFree)
	{
		queryToFree = toLoop->next;
		free(toLoop->line);
		for(int i = 0; i < toLoop->numberOfValues; i++)
			free(toLoop->values[i]);
		free(toLoop->values);
		free(toLoop);
	}
}

/*Alloction of rawData structure*/
rawData* mallocRawData(int rows, int cols)
{
	rawData* toReturn = (rawData*)malloc(sizeof(rawData));
	toReturn->data = (char***)malloc(rows * sizeof(char**));
	for(int i = 0; i < rows; i++)
	{
		toReturn->data[i] = (char**)malloc(cols * sizeof(char*));
		for(int j = 0; j < cols; j++)
			toReturn->data[i][j] = (char*)malloc(VALUESSIZE * sizeof(char*));
	}

	toReturn->rows = rows;
	toReturn->cols = cols;
	toReturn->previous = NULL;
	toReturn->next = NULL;

	return toReturn;
}

/*Frees the rawData structure*/
void freeRawData(rawData* toFree)
{
	rawData* toLoop = toFree;
	while(toLoop != NULL)
	{
		toFree = toLoop->next;
		for(int i = 0; i < toLoop->rows; i++)
		{
			for(int j = 0; j < toLoop->cols; j++)
				free(toLoop->data[i][j]);

			free(toLoop->data[i]);
		}
		free(toLoop->data);
		free(toLoop);
		toLoop = toFree;
	}
}

/*Appends rawData to the structure
 * rawData is a linkedList*/
rawData* appendRawData(rawData* mainData, rawData* toAppend)
{
	mainData->next = toAppend;
	toAppend->previous = mainData;

	return mainData;
}

/*Executes queries of change on the database
 * writes, updates, deletes*/
int executeQueryOnDatabase(queryText* queries)
{
	char* conninfo = getenv("CONNINFO");
	PGconn* conn = PQconnectdb(conninfo);
	queryText* toLoop = queries;

	if(PQstatus(conn) == CONNECTION_OK)
	{
		PGresult* res = PQexecParams(conn, toLoop->line, toLoop->numberOfValues, NULL, (const char**)toLoop->values, NULL, NULL, 0);
		ExecStatusType resStatus = PQresultStatus(res);
		if(resStatus != PGRES_COMMAND_OK)
			return -1;
		PQclear(res);
		toLoop = toLoop->next;
	}
	freeTextQueries(queries);
	PQfinish(conn);
	return 0;
}

/*Executes queries that dont change the database
 * read*/
rawData* executeReadingOnDataBase(queryText* queries)
{
	char* conninfo = getenv("CONNINFO");
	PGconn* conn = PQconnectdb(conninfo);
	queryText* toLoop = queries;
	rawData* myData = NULL;

	if(PQstatus(conn) == CONNECTION_OK)
	{
		while(toLoop != NULL)
		{
			PGresult* res = PQexecParams(conn, toLoop->line, toLoop->numberOfValues, NULL, (const char**)toLoop->values, NULL, NULL, 0);
			ExecStatusType resStatus = PQresultStatus(res);
			if(resStatus == PGRES_TUPLES_OK)
			{
				rawData* dataToAppend = mallocRawData(PQntuples(res), PQnfields(res));
				for(int i = 0; i < dataToAppend->rows; i++)
				{
					for(int j = 0; j < dataToAppend->cols; j++)
						strcpy(dataToAppend->data[i][j], PQgetvalue(res, i, j));
				}

				if(myData == NULL)
					myData = dataToAppend;
				else
					appendRawData(myData, dataToAppend);
			}
			PQclear(res);	
            toLoop = toLoop->next;
		} 

	} else
		return NULL;

    printf("here in C\n");
	freeTextQueries(queries);
	PQfinish(conn);
	return myData;
}
