#pragma once
#include <cctype>
#include <map>
#include <random>
#include <string>
#include <utility>

namespace seeds::life {

struct GeneDefinition {
    std::string name;
    std::string dominant_trait;
    std::string recessive_trait;
};

struct AllelePair {
    std::string first;
    std::string second;

    [[nodiscard]] bool dominant_present() const {
        return (!first.empty() && std::isupper(static_cast<unsigned char>(first[0]))) ||
               (!second.empty() && std::isupper(static_cast<unsigned char>(second[0])));
    }

    [[nodiscard]] std::pair<std::string, std::string> inherited_alleles() const {
        return {first, second};
    }
};

struct Genome {
    std::map<std::string, AllelePair> genes;
    double mutation_rate = 0.02;
};

struct Phenotype {
    std::map<std::string, std::string> traits;
    double adaptation_score = 0.0;
    int dominant_gene_count = 0;
};

inline const std::map<std::string, GeneDefinition>& default_gene_library() {
    static const std::map<std::string, GeneDefinition> lib = {
        {"speed",      {"speed",      "swift",     "steady"}},
        {"resilience", {"resilience", "hardy",     "fragile"}},
        {"fertility",  {"fertility",  "fecund",    "sparse"}},
        {"camouflage", {"camouflage", "concealed", "visible"}},
    };
    return lib;
}

class GeneticsEngine {
public:
    explicit GeneticsEngine(std::mt19937& rng) : rng_(rng) {}

    Genome create_founder(double mutation_rate = 0.02) {
        Genome genome;
        genome.mutation_rate = mutation_rate;
        std::uniform_int_distribution<int> dist(0, 1);
        for (const auto& [gene_name, _] : default_gene_library()) {
            std::string a1 = dist(rng_) ? "A" : "a";
            std::string a2 = dist(rng_) ? "A" : "a";
            genome.genes[gene_name] = AllelePair{a1, a2};
        }
        return genome;
    }

    Phenotype express(const Genome& genome) {
        Phenotype phenotype;
        int dominant_count = 0;
        for (const auto& [gene_name, pair] : genome.genes) {
            const auto& def = default_gene_library().at(gene_name);
            if (pair.dominant_present()) {
                phenotype.traits[gene_name] = def.dominant_trait;
                ++dominant_count;
            } else {
                phenotype.traits[gene_name] = def.recessive_trait;
            }
        }
        phenotype.dominant_gene_count = dominant_count;
        phenotype.adaptation_score = static_cast<double>(dominant_count) /
                                     std::max(1, static_cast<int>(genome.genes.size()));
        return phenotype;
    }

    Genome crossover(const Genome& first, const Genome& second) {
        Genome child;
        child.mutation_rate = std::max(first.mutation_rate, second.mutation_rate);
        std::uniform_int_distribution<int> dist(0, 1);
        for (const auto& [gene_name, _] : default_gene_library()) {
            const auto& pair_a = first.genes.at(gene_name);
            const auto& pair_b = second.genes.at(gene_name);
            auto [a1, a2] = pair_a.inherited_alleles();
            auto [b1, b2] = pair_b.inherited_alleles();
            std::string first_allele = dist(rng_) ? a2 : a1;
            std::string second_allele = dist(rng_) ? b2 : b1;
            child.genes[gene_name] = AllelePair{
                mutate_allele(first_allele, child.mutation_rate),
                mutate_allele(second_allele, child.mutation_rate),
            };
        }
        return child;
    }

private:
    std::string mutate_allele(const std::string& allele, double mutation_rate) {
        std::uniform_real_distribution<double> dist(0.0, 1.0);
        if (dist(rng_) >= mutation_rate) return allele;
        std::string result = allele;
        for (char& c : result) {
            if (std::isupper(static_cast<unsigned char>(c)))
                c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
            else
                c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        return result;
    }

    std::mt19937& rng_;
};

} // namespace seeds::life
