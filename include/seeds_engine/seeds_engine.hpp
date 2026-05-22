#pragma once

// Seeds Engine v1.0.0 — C++20 ecology simulation library (shared/dynamic)

#include "core/time.hpp"
#include "core/events.hpp"
#include "core/scheduler.hpp"
#include "core/engine.hpp"

#include "world/biome.hpp"
#include "world/terrain.hpp"
#include "world/climate.hpp"
#include "world/resources.hpp"
#include "world/map.hpp"

#include "life/species.hpp"
#include "life/genetics.hpp"
#include "life/organism.hpp"
#include "life/aging.hpp"
#include "life/disease.hpp"
#include "life/faith.hpp"
#include "life/physiology.hpp"
#include "life/reproduction.hpp"

#include "behavior/actions.hpp"
#include "behavior/perception.hpp"
#include "behavior/utility_ai.hpp"
#include "behavior/social.hpp"

#include "ecology/competition.hpp"
#include "ecology/foodweb.hpp"
#include "ecology/migration.hpp"
#include "ecology/population.hpp"
#include "ecology/predation.hpp"

#include "storage/snapshots.hpp"
#include "storage/repository.hpp"
#include "storage/formats.hpp"

#include "ai/anomaly.hpp"
#include "ai/clustering.hpp"
#include "ai/prediction.hpp"
#include "ai/reports.hpp"

#include "debug/debugger.hpp"
#include "net/mcp_server.hpp"

#include "api/schemas.hpp"
#include "api/facade.hpp"
