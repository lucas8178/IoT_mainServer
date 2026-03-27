#include "../../include/database/database.h"
#include "../../include/globalStructures/globalStructures.h"

#define VALUESQUANTITY 1
typedef struct sensors
{
    uint8_t id;
	uint32_t minMoisture;
	uint32_t maxMoisture;
	uint32_t maxExternalTemperature;
	uint32_t minExternalTemperature;
	uint32_t maxInternalTemperature;
	uint32_t minInternalTemperature;
	uint8_t plantId;
	struct sensors* previous;
	struct sensors* next;
} sensors;

typedef enum searchType
{
	ID, END
} searchType;

typedef queryText* (searchPlantsQueryGeneration)(sensors*, searchType*);

sensors* rawDataToSensors(rawData* myData);
sensors* mallocSensors();
void freeSensors(sensors* toFree);
queryText* searchSensorsQuery(sensors* toSearch, searchType* searchKind);
sensors* readPlants(sensors* toSearch, searchType* searchKind, searchPlantsQueryGeneration query);
