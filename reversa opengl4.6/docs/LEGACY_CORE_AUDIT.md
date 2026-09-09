# Auditoria de reachability do legado

O executável contém `H04CompatibilityFixedStateBypass`, `H05CoreProfile` e strings de auditoria para categorias de fixed-function. As categorias encontradas incluem immediate mode, matrix stack, fixed capability, fixed fog, texture environment, client array, attribute stack, removed query, quads draw e current color.

A intenção observável é permitir um caminho Compatibility enquanto se mede e elimina dependências até o Core profile. `LegacyReachabilityAudit` reproduz o contador clean-room para uso no nosso projeto.
