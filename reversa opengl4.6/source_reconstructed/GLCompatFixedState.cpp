// RECONSTRUÇÃO documental de GLCompatFixedState.cpp.
// Strings literais do binário mostram as categorias auditadas do fixed-function.
namespace reverse_gl46 {

enum class LegacyCategory {
    ImmediateMode,
    MatrixStack,
    FixedCapability,
    FixedFog,
    TextureEnvironment,
    ClientArray,
    AttributeStack,
    RemovedQuery,
    QuadsDraw,
    CurrentColor,
};

// O binário escreve GLLegacyReachability.log com:
// stage=h04_legacy_reachability
// renderer_mode, renderer_name, site_capacity, reached_sites, total_hits,
// category_*, suppressed_*, suppressed_total e lista de sites alcançados.
// Isso indica uma camada de compatibilidade instrumentada para descobrir
// quais caminhos legacy ainda são executados enquanto o renderer moderno é ativado.

} // namespace reverse_gl46
