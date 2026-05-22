#pragma once
#include <map>
#include <string>
#include <variant>
#include <vector>

#include "anomaly.hpp"
#include "clustering.hpp"
#include "prediction.hpp"
#include "../ecology/population.hpp"

namespace seeds::ai {

using ReportValue = std::variant<int, std::string, std::map<std::string, double>,
                                 std::vector<AnomalyFinding>, std::map<std::string, std::string>,
                                 ecology::PopulationStats>;
using AnalyticsReport = std::map<std::string, ReportValue>;

class AnalyticsReportService {
public:
    template <typename Engine>
    AnalyticsReport summary(Engine& engine) const {
        AnalyticsReport report;
        report["tick"] = engine.clock.tick();
        report["season"] = engine.clock.season();
        report["carrying_capacity"] = predictor_.predict(engine);
        report["anomalies"] = detector_.detect(engine);
        report["species_clusters"] = clusterer_.cluster(engine);
        return report;
    }

private:
    CarryingCapacityPredictor predictor_;
    AnomalyDetector detector_;
    SpeciesClusterer clusterer_;
};

} // namespace seeds::ai
