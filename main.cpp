#include <bits/stdc++.h>
#include <curl/curl.h>
#include "Utility/Logging.hpp"
#include "Utility/HttpSession.hpp"
#include "Utility/Configuration.hpp"
#include "Abstract/EndpointListener.hpp"
#include "Listeners/Cpi/CpiRssEndpointListener.hpp"
#include "Listeners/Cpi/CpiPdfEndpointListener.hpp"
#include "Listeners/Cpi/CpiHtmEndpointListener.hpp"
#include "Listeners/Cpi/CpiApiEndpointListener.hpp"
#include "Utility/TcpServer.hpp"
#include "Utility/SignalHandler.hpp"

#include <P7/P7_Trace.h>

using namespace std;

using namespace Logging;
using namespace Logging::IP7_Targets;

unique_ptr<TcpServer> tcpServer;

auto MakeMatchHandler(const string &identifier) {
    return [identifier](double value) {
        int64_t timestamp = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        LOG(INFO, TM("$AT %ld MATCH VALUE %f FROM: %hs"), timestamp, value, identifier.c_str());
        char message[256];
        sprintf(message, R"({ "value": %f, "identifier": "%s", "timestamp": %ld })", value, identifier.c_str(), timestamp);
        cerr << message << "\n";
        *(int64_t*)message = timestamp;
        *(double*)(message + 8) = value;
        *(int*)(message + 16) = 10;
        memset(message + 20, NULL, 12);
        memcpy(message + 20, identifier.c_str(), identifier.size());
        tcpServer->SendToAll(message);
    };
}

unique_ptr<vector<char>> ReadFileBytes(const string &filename) {
    unique_ptr<vector<char>> data = make_unique<vector<char>>();
    ifstream in(__CPI_TEST_DATA_DIRECTORY_PATH + "/"s + filename);
    while (in) {
        data->push_back(in.get());
    }
    return data;
}

shared_ptr<Configuration> Configure(int argc, const char **argv) {
    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--config")) {
            return Configuration::Parse(argv[i + 1]);
        }
    }
    return Configuration::Parse(__CONFIGURATION_FILE_PATH);
}

int main(int argc, const char **argv) {
    
    signal(SIGTERM, SignalHandler::HandleSignal);

    bool test = false;
    bool info = false;
    bool testwatch_activator = false;
    int port = -1;

    for (int i = 1; i < argc; ++i) {
        test = test || !strcmp(argv[i], "--test");
        info = info || !strcmp(argv[i], "--info");
        testwatch_activator = testwatch_activator || !strcmp(argv[i], "--testwatch-activator");
        if (!strcmp(argv[i], "--port")) {
            port = stoi(argv[i + 1]);
        }
    }
    
    if (info) {
        puts("");
        puts("configuration file:");
        puts(">\t" __CONFIGURATION_FILE_PATH);
        puts("");
        puts("logs directory:");
        puts(">\t" __LOGS_DIRECTORY_PATH);
        puts("");
        puts("test logs directory:");
        puts(">\t" __TEST_LOGS_DIRECTORY_PATH);
        puts("");
        puts("testwatch activator file:");
        puts(">\t" __TESTWATCH_ACTIVATOR_FILE_PATH);
        puts("");
        puts("CPI test data directory:");
        puts(">\t" __CPI_TEST_DATA_DIRECTORY_PATH);
        puts("");
        return 0;
    }
    
    if (testwatch_activator) {
        system("$(ls /bin | grep -m1 -o -E '^python3?(\\.[[:digit:]]+)?$') " __TESTWATCH_ACTIVATOR_FILE_PATH);
        return 0;
    }

    if (port < 0) {
        port = test ? 17111 : 11111;
    }

    Logger::Initialize(test);

    tcpServer = make_unique<TcpServer>(port);

    if (test) {

        auto apiUrl = "https://api.bls.gov/publicAPI/v2/timeseries/data/CUUR0000SA0?latest=true&registrationkey=6ac053225190425593d38b3a67e24555";
        auto apiTestFilename = "cpi_api_test.dat";
        auto apiMatchHandler = MakeMatchHandler("cpi/api");
        auto apiEndpointListener = CpiApiEndpointListener(apiUrl, apiMatchHandler);
        {
            auto data = ReadFileBytes(apiTestFilename);
            apiEndpointListener.Watch(1500, std::move(data));
        }

        auto htmUrl = "https://www.bls.gov/news.release/cpi.nr0.htm";
        auto htmTestFilename = "cpi_htm_test.dat";
        auto htmMatchHandler = MakeMatchHandler("cpi/htm");
        auto htmEndpointListener = CpiHtmEndpointListener(htmUrl, htmMatchHandler);
        {
            auto data = ReadFileBytes(htmTestFilename);
            htmEndpointListener.Watch(1000, std::move(data));
        }

//    auto pdfUrl = "https://www.bls.gov/news.release/pdf/cpi.pdf";
//    auto pdfMatchHandler = MakeMatchHandler("cpi/pdf");
//    auto pdfEndpointListener = CpiPdfEndpointListener.Create(pdfUrl, pdfMatchHandler);
//    pdfEndpointListener.Watch(1000);

        auto rssUrl = "https://www.bls.gov/feed/bls_latest.rss";
        auto rssTestFilename = "cpi_rss_test.dat";
        auto rssMatchHandler = MakeMatchHandler("cpi/rss");
        auto rssEndpointListener = CpiRssEndpointListener(rssUrl, rssMatchHandler);
        {
            auto data = ReadFileBytes(rssTestFilename);
            rssEndpointListener.Watch(1000, std::move(data));
        }

        tcpServer->Listen(128);

    }

    auto configuration = Configure(argc, argv);

    vector<shared_ptr<EndpointListener>> listeners;
    for (auto &i: configuration->GetListeners()) {
        string identifier = i->GetIdentifier();
        assert(identifier.starts_with("cpi"));
        string source = identifier.substr(4);
        if (source == "api") {
            string postfix = i->GetApiKey().has_value() ? "&registrationkey=" + i->GetApiKey().value() : "";
            auto apiMatchHandler = MakeMatchHandler("cpi/api");
            listeners.push_back(shared_ptr<EndpointListener>(new CpiApiEndpointListener(i->GetUrl() + postfix, apiMatchHandler)));
        } else if (source == "htm") {
            auto htmMatchHandler = MakeMatchHandler("cpi/htm");
            listeners.push_back(shared_ptr<EndpointListener>(new CpiHtmEndpointListener(i->GetUrl(), htmMatchHandler)));
        } else if (source == "pdf") {
            auto pdfMatchHandler = MakeMatchHandler("cpi/pdf");
            listeners.push_back(shared_ptr<EndpointListener>(new CpiPdfEndpointListener(i->GetUrl(), pdfMatchHandler)));
        } else if (source == "rss") {
            auto rssMatchHandler = MakeMatchHandler("cpi/rss");
            listeners.push_back(shared_ptr<EndpointListener>(new CpiRssEndpointListener(i->GetUrl(), rssMatchHandler)));
        } else {
            throw exception();
        }
        for (int j = 0; j < i->GetCount(); ++j) {
            listeners.rbegin()->get()->Watch(i->GetMinDelay());
        }
    }

    tcpServer->Listen(128);

}
