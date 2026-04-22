struct appSave {
    char* appName;
    size_t sectorOffset;
    bool alsoMap;

    size_t currentSectorsWritten;
    size_t idealBytesWritten;
    uint8_t* cache;
    size_t cacheSize;

    void* returnVal;
};

struct memdiffGenerator {
    uint8_t* a;
    uint8_t* b;
    int index;
    int target;
    int differenceCount;

    int runBeginning;
    int runEnding;
    bool runGoing;

    int maxPrint;
    int printed;
};
