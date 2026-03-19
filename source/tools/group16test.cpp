#include "AnalyticsManager.hpp"
#include "DataLog.hpp"
#include "ActionLog.hpp"
int main(){
    cse498::AnalyticsManager analytics;
    analytics.LogHealth(100.0);
    analytics.LogEnemiesTracked(5);
    analytics.LogDamageDealt(50.0);

    analytics.LogHealth(80.0);
    analytics.LogEnemiesTracked(3);
    analytics.LogDamageDealt(25.0);

    analytics.LogHealth(100.0);
    analytics.LogEnemiesTracked(1);
    analytics.LogDamageDealt(50.0);

    return 0;
}