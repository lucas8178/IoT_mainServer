#include "../../include/database/sensors.h"

/*Conversion of rawData to Sensors data*/
sensors* rawDataToSensors(rawData* myData)
{
	sensors* toReturn = NULL;
	sensors* mainSensor = NULL;
	int i = 0;
	for(rawData* toLoop = myData; toLoop != NULL; toLoop = toLoop->next)
	{
		sensors* newSensor = mallocSensors();
		newSensor->id = strtol(myData->data[i][0], NULL, 10);
		newSensor->minMoisture = strtol(myData->data[i][1], NULL, 10);
		newSensor->maxMoisture = strtol(myData->data[i][2], NULL, 10);
		newSensor->maxExternalTemperature = strtol(myData->data[i][3], NULL, 10);
		newSensor->minExternalTemperature = strtol(myData->data[i][4], NULL, 10);
		newSensor->maxInternalTemperature = strtol(myData->data[i][5], NULL, 10);
		newSensor->minInternalTemperature = strtol(myData->data[i][6], NULL, 10);
        newSensor->plantId = strtol(myData->data[i][7], NULL, 10);

		if(toReturn == NULL)
		{
			toReturn = newSensor;
			mainSensor = newSensor;
		}
		else
		{
			for(mainSensor; mainSensor->next != NULL; mainSensor = mainSensor->next);
			mainSensor->next = newSensor;
			newSensor->previous = mainSensor;
		}
		i++;
	}

	freeRawData(myData);
	return toReturn;
}

/*Allocation of sensors* structure*/
sensors* mallocSensors()
{
	sensors* mySensors = (sensors*)malloc(sizeof(sensors));
	mySensors->previous = NULL;
	mySensors->next = NULL;

	return mySensors;
}

/*Frees sensors datas*/
void freeSensors(sensors* toFree)
{
	sensors* toLoop = toFree;
	
	while(toLoop != NULL)
	{
		toFree = toLoop->next;
		free(toLoop);
		toLoop = toFree;
	}
}

/*Creates the query to be executed in the database*/
queryText* searchSensorsQuery(sensors* toSearch, searchType* searchKind)
{
	sensors* toLoop = NULL;
	queryText* toReturn = NULL;
	int queriesNumber = 0;
	int i = 0;

	for(toLoop = toSearch; toLoop != NULL; toLoop = toLoop->next)
	{
		queriesNumber++;
	}

	toReturn = mallocTextQueries(queriesNumber, VALUESQUANTITY);

	toLoop = toSearch;
	for(queryText* queriesToLoop = toReturn; queriesToLoop != NULL; queriesToLoop = queriesToLoop->next)
	{
		if(searchKind[i] == ID)
		{
			sprintf(queriesToLoop->line, "SELECT * FROM sensors WHERE plantId = $1;");
			sprintf(queriesToLoop->values[0], "%d", toLoop->plantId);
			toLoop = toLoop->next;
		}	
		i++;
	}

	freeSensors(toSearch);
	return toReturn;
}

/*executes the query and return the values finded*/
sensors* readPlants(sensors* toSearch, searchType* searchKind, searchPlantsQueryGeneration query)
{
	queryText* myQuery = query(toSearch, searchKind);
	return rawDataToSensors(executeReadingOnDataBase(myQuery));
}

