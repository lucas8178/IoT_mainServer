
typedef struct currentDate
{
    int day: 5;
    int month: 4;
    int year 12;
} currentDate;

typedef struct situationData
{
    int8_t id;
    int8_t plantid;
    float internalTemperature;
    float externalTemperature;
    currentDate date;
} situationData;
