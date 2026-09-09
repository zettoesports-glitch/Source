# Renderer2DGL46

O binário nomeia D01..D14 e métricas de `submitted_quads`, `flush_calls`, `draw_calls`, `barrier_calls`, capacity flushes, stream uploads e máximos por draw. A arquitetura observada é uma fila de quads com batching por estado/textura e barreiras de ordenação.

Text surfaces possuem cache com hits/misses/evictions; nomes acima dos personagens têm P01HeadNameTextReuse. HP bars de monstros possuem batch dedicado. Partículas possuem deferred texture bind e direct Z rotation.

`Renderer2DBatcher` e `TextSurfaceCache` são reconstruções clean-room dessas decisões, não os tipos originais.
