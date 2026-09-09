# 30 — Correspondência da revisão pública com o executável

Status: **CONFIRMADO / correspondência temporal extremamente forte**.

## Executável analisado

`NextMU.exe` x64 do pacote `Resources_DE_2024-01-25` possui timestamp PE:

```text
2024-01-25 01:55:21 UTC
```

## Revisão pública encontrada

Repositório público:

```text
kuncarous/nextmu
```

Commit:

```text
b0388a6416379c03dc9089d74342180eb2560042
```

Mensagem:

```text
Updated main menu
```

Timestamp do commit:

```text
2024-01-25 01:55:35 UTC
```

Diferença para o timestamp PE: **14 segundos**.

## Interpretação

Essa proximidade, combinada com os mesmos nomes de source preservados no binário e com a correspondência dos layouts, buffers, programas, uniforms e chamadas Diligent já reconstruídos antes de localizar a source, constitui evidência muito forte de que essa revisão é a source usada para gerar o executável ou uma revisão imediatamente posterior sem alteração relevante no renderer.

Não tratamos a igualdade byte-a-byte como provada sem rebuild reproduzível, mas a revisão passa a ser a principal referência para a camada NextMU.

## Licença

O repositório público usa licença MIT. Qualquer cópia substancial de código deve preservar os avisos de licença/copyright aplicáveis.

## Estratégia da engenharia daqui em diante

```text
source pública próxima/exata
        +
provas do binário fornecido
        ↓
CONFIRMADO POR SOURCE + BINÁRIO
```

Quando a source revelar nomes que antes estavam apenas reconstruídos, os documentos antigos devem ser corrigidos para os nomes reais.
