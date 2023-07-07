#include "x2struct/x2struct.hpp"
#include <iostream>
#include "common/singleton.hpp"
#include "common/log/log.h"
#include "common/ASFramework/util/ASLogImpl.h"
#include "common/debug/top_tools.h"
#include "common/debug/time_debug.h"

using namespace std;
using namespace x2struct;

static std::string strChartName = "x2structUnitTest";
static unsigned timeIntervalSeconds = 1;

enum Mode
{
    MASTER = 1,
    SLAVE = 2,
};

struct condition
{
    int id;
    string url;
    XTOSTRUCT(M(url));
};

#ifdef X_SUPPORT_C0X
struct sub
{
    int a;
    string b;
};
XTOSTRUCT_OUT(sub, M(a), O(b));
#else
struct sub
{
    int a;
    string b;
    XTOSTRUCT(M(a), O(b));
};
#endif

struct SharePtr
{
    int a;
    XTOSTRUCT(O(a));
};

struct xstruct
{
    int id;
    XDate start;
    int tint;
    string tstring;
    char chArray[16];
#ifdef X_SUPPORT_C0X
    std::shared_ptr<SharePtr> sp;
    std::unordered_map<std::string, int> umap;
#else
    SharePtr sp;
    std::map<std::string, int> umap;
#endif
    vector<int> vint;
    list<int> lint;
    vector<string> vstring;
    vector<int64_t> vlong;
    vector<sub> vsub;
    vector<vector<int>> vvint;
    vector<vector<string>> vvstring;
    vector<vector<sub>> vvsub;
    map<int, sub> tmap;
    condition con;
    Mode md;
    XTOSTRUCT(A(id, "config:id _id,m"), C(con), O(tint, tstring, sp, vint, vstring, vlong, vsub, vvint, vvstring, vvsub, tmap, con));
    XTOSTRUCT_CONDITION(xstruct, con)
    {
        int _id;
        return obj.convert("id", _id) && _id == this->id;
    }
};

class GetOptArgs {
public:
    int m_count;
    GetOptArgs():m_count(1) {}
    void Usage(const char *name);
    bool GetOpt(int & argc, char* argv[]);
};

class UnitTest : public GetOptArgs {
public:
    UnitTest() {}
    virtual ~UnitTest();

    bool Init(int & argc, char* argv[]);
    std::ofstream m_dot_file;
};

void GetOptArgs::Usage(const char *name) {
    fprintf(stderr, "input argv: %s \
\n -c <count>\n", name);
}

bool GetOptArgs::GetOpt(int & argc, char* argv[]){
    const char *args = "hc::";
    extern char *optarg;
    int rt;
    while ((rt = getopt(argc, argv, args)) != -1) {
        switch (rt) {
            case 'h':
                Usage(argv[0]);
                return false;
            case 'c':
                m_count = atoi(optarg);
                break;
            default:
                Usage(argv[0]);
                return false;
        }
    }
    return true;
}

bool UnitTest::Init(int & argc, char* argv[]) {
    if (!GetOpt(argc, argv)) {
        return false;
    }

    std::string strDotPath = "./dot.txt";
    m_dot_file.open(strDotPath.c_str(), std::ios::out | std::ios::binary | std::ios::app);
    if (!m_dot_file) {
        LOG_ERROR("write dot info into file[%s] failed, error(%d:%s).", strDotPath.c_str(), errno, strerror(errno));
        return false;
    }
    return true;
}

UnitTest::~UnitTest() {
    if (m_dot_file) {
        m_dot_file.close();
        m_dot_file.clear();
    }
}

int main(int argc, char* argv[])
{
    // init log
    std::string strLogPath = "./" + strChartName + ".log";
    CASLogImpl* pLogger = new (std::nothrow) CASLogImpl();
    pLogger->AddRef();
    pLogger->SetLogFilePath(strLogPath.c_str());
    pLogger->SetLogMaxSize(10 * 1024 * 1024);
    pLogger->SetLogLevel(ASLog_Level_Debug);
    pLogger->Open();
    CEntModuleDebug::SetModuleDebugger(pLogger);
    LOG_DEBUG("<------------------------------------>");
    LOG_DEBUG("---> init %s log success <---", strChartName.c_str());

    UnitTest unittest;
    if (!unittest.Init(argc, argv)) {
        return -1;
    }
    GENERATE_LOCAL_TIME_DEBUG

    unittest.m_dot_file << "DOT BEGIN" << "\n";

    std::string htmlname = "./" + strChartName + ".html";
    TOP_TOOLS_START(htmlname, timeIntervalSeconds)

    for (int i = 0; i < unittest.m_count; i++) {
        TIME_DEBUG_START
        xstruct x;
        cout << "======== struct <-----> string ===========" << endl;
        X::loadjson("test.json", x, true);
        cout << X::tojson(x) << endl << endl;
        #ifdef DEBUG
        {
            unsigned long time = 0;
            int code = time_debug.GetTimeFromLast(time, true);
            if (code != TIME_DEBUG_OK) {
                LOG_DEBUG("GetTimeFromLast failed with code %d", code);
            } else {
                unittest.m_dot_file << "count = " << unittest.m_count;
                unittest.m_dot_file << ", current = " << i;
                unittest.m_dot_file << ", used[" << time << "]us;\n";
            }
        }
        #endif
        TIME_DEBUG_STOP
        usleep(100 * 1000);
    }
    TOP_TOOLS_STOP
    return 0;
}
 
 
