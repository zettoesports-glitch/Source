# AUDITORIA MESTRE DO PROJETO COMPLETO

Branch auditada: `modernization`

Data da auditoria: 2026-09-07

## 1. Objetivo

Este documento consolida o estudo arquitetural do repositório `zettoesports-glitch/Source` como um sistema completo, e não somente como um cliente gráfico.

A finalidade é definir com precisão:

- quais componentes formam o runtime do MU;
- como cliente, servidores, banco, launcher, updater, proteção, Lua e dados se conectam;
- quais contratos precisam permanecer byte-for-byte ou semanticamente compatíveis;
- quais áreas podem ser modernizadas de forma isolada;
- quais áreas não devem ser alteradas durante a modernização gráfica;
- onde o renderer moderno OpenGL/Vulkan deve se encaixar sem contaminar gameplay, protocolo, persistência ou proteção.

### Legenda de classificação

- **NÃO TOCAR** — alto risco de alterar gameplay, protocolo, persistência, proteção ou ABI.
- **PRESERVAR CONTRATO** — pode receber refatoração interna, mas formato/semântica externa deve permanecer idêntico.
- **MODERNIZAR ISOLADO** — alvo adequado para modernização atrás de uma interface clara.
- **DEPLOYMENT/TOOLS** — ferramenta operacional; não faz parte diretamente do frame/gameplay.
- **LEGADO ISOLÁVEL** — mecanismo antigo que deve continuar funcional enquanto o substituto é validado.

---

## 2. Visão executiva

O repositório é um ecossistema completo com múltiplos executáveis e grandes domínios acoplados:

```text
SOURCE
|
+-- Main ---------------------------- Cliente MU
|   +-- Win32 / Window / Input
|   +-- Network / Protocol
|   +-- Scene / World / Character
|   +-- BMD / Animation / Physics
|   +-- Terrain / Effects / UI
|   +-- OpenGL legado
|   +-- CoreGLCompat
|   +-- Renderer moderno em construção
|   +-- Lua cliente / customs
|   +-- MHP integrado
|   +-- In-game updater
|
+-- ConnectServer ------------------- Descoberta/lista de servidores
+-- JoinServer ---------------------- Autenticação e sessão de conta
+-- GameServer ---------------------- Gameplay autoritativo
+-- DataServer ---------------------- Persistência/banco
|
+-- MHP-Server - VS 2010 ------------ Anti-hack client/server/verify
+-- GetMainInfo --------------------- Gerador do contrato binário info.bmd
+-- Launcher C# - WPF --------------- Launcher/updater/configuração
+-- MainUpdater --------------------- Empacotador de atualizações
+-- MuServer ------------------------ Runtime/configuração/SQL/deployment
|
+-- vulkan-main/.../Shaders --------- Fonte HLSL de referência
+-- opengl-main/.../OpenGL ---------- GLSL gerado
+-- graphify-out -------------------- Grafo de navegação do código
```

A conclusão principal é que **a modernização gráfica deve permanecer uma ilha arquitetural dentro de `Main`**. Não há justificativa técnica para alterar GameServer, DataServer, JoinServer, ConnectServer, protocolos, formatos BMD, `MAIN_FILE_INFO`, banco ou MHP para implementar o renderer moderno.

A arquitetura segura continua sendo:

```text
MU LOGIC / GAME STATE
        |
        v
RENDER FRONTEND
        |
        v
RHI
   +----+----+
   |         |
 OpenGL    Vulkan
```

O backend OpenGL moderno deve ser validado primeiro. Vulkan permanece posterior à estabilização do frontend/RHI.

---

## 3. Inventário dos componentes principais

| Componente | Papel | Classificação para a modernização gráfica |
|---|---|---|
| `Main` | Cliente completo | separar renderer do restante |
| `ConnectServer` | lista/roteamento/liveness de servidores | **NÃO TOCAR** |
| `JoinServer` | autenticação e sessão de conta | **NÃO TOCAR** |
| `GameServer` | gameplay autoritativo | **NÃO TOCAR** |
| `DataServer` | persistência e serviços DB | **NÃO TOCAR** |
| `MuServer/Data` | regras/configs server-side | **PRESERVAR CONTRATO** |
| `GetMainInfo` | gera `MAIN_FILE_INFO` criptografado | **PRESERVAR CONTRATO BINÁRIO** |
| `MHP-Server - VS 2010` | proteção/anti-hack | **NÃO TOCAR** |
| `Launcher C# - WPF` | update/config/start | **DEPLOYMENT/TOOLS** |
| `MainUpdater` | geração de ZIPs de update | **DEPLOYMENT/TOOLS** |
| `vulkan-main/Shaders` | HLSL de referência | **PRESERVAR COMO FONTE DE VERDADE** |
| `opengl-main/OpenGL` | GLSL gerado | **ARTEFATO GERADO** |
| `CoreGLCompat` | emulação/shim do legado no Core Profile | **LEGADO ISOLÁVEL** |
| `Render/*` novo | frontend/RHI/backends novos | **MODERNIZAR ISOLADO** |

---

## 4. Fluxo operacional dos servidores

O deployment em `MuServer/StartUp/Configuration/Configuration.xml` confirma a ordem operacional:

```text
ConnectServer
    |
    v
DataServer
    |
    v
JoinServer
    |
    v
GameServer
```

A configuração atual introduz aproximadamente 1 segundo entre cada processo.

### 4.1 ConnectServer

Responsabilidades observadas:

- TCP público, normalmente na porta 44405;
- UDP para comunicação de estado/liveness com servidores;
- lista de GameServers em `Data/ServerList.dat`;
- limite de conexões por IP;
- geração e entrega da server list;
- acompanhamento de servidores ativos.

Classificação: **NÃO TOCAR** durante a modernização gráfica.

### 4.2 JoinServer

Responsabilidades observadas:

- autenticação da conta;
- manutenção da sessão da conta conectada;
- prevenção de dupla conexão;
- AccountLevel e expiração;
- map-server move / autenticação de transferência;
- comunicação com o banco via ODBC;
- opção de senha comum ou MD5 conforme configuração.

`JoinServerProtocolCore()` trata os headers de conexão, desconexão, move entre servidores, autorização e AccountLevel.

Classificação: **NÃO TOCAR**.

### 4.3 GameServer

É a autoridade de gameplay.

`GameMainInit()` inicializa, entre outros:

```text
gObj
HackCheck
BattleSoccer
ServerInfo
Experience Table
MonsterManager
LuaGameServer
JoinServer connection
DataServer connection
```

Depois o GameServer executa timers autoritativos para:

- monster processing;
- monster movement;
- monster AI;
- event processing;
- viewport;
- object lifecycle;
- account level;
- authentication;
- pick command.

O callback desses timers usa uma `CCriticalSection`, ou seja, o runtime já impõe uma forma de serialização entre esses jobs.

A árvore do GameServer contém módulos de grande porte como:

- `Attack`;
- `BloodCastle`;
- `CastleSiege`;
- `ChaosBox`;
- `CashShop`;
- quests;
- Gens;
- Muun;
- events;
- inventory;
- skills;
- personal shop;
- monster AI;
- custom systems.

Classificação: **NÃO TOCAR** para o projeto de renderer.

### 4.4 DataServer

O DataServer não é apenas save/load de personagem. `DataServerProtocolCore()` é uma central de persistência para:

- character list/create/delete/info;
- warehouse;
- item creation;
- options;
- pet data;
- quests;
- master skill;
- NPC helper/Santa;
- resets/master resets;
- QuestWorld;
- Gens;
- MuRummy;
- Helper;
- CashShop;
- e outros domínios condicionados pela versão de build.

O entrypoint conecta diretamente a ODBC e inicia o socket TCP do DataServer.

Classificação: **NÃO TOCAR**.

---

## 5. Fluxo de rede fim a fim

Visão conceitual:

```text
Launcher
   |
   v
Main.exe
   |
   +--> ConnectServer ------ server list / rota
   |
   v
GameServer
   |
   +--> JoinServer --------- autenticação/sessão
   |
   +--> DataServer --------- persistência
   |
   +--> ConnectServer UDP -- liveness/user count
```

### 5.1 Cliente clássico

`Main/source/Protocol.cpp` preserva o protocolo C1/C2/C3/C4 e o fluxo de criptografia/serial para C3/C4.

`cProtocol::DataSend()` usa `PacketManager`, aplica encryption quando necessário e envia pelo socket do cliente.

Isso é uma fronteira de compatibilidade: **não alterar layout, serialização ou ordenação dos packets durante o trabalho do renderer**.

### 5.2 `NEW_PROTOCOL_SYSTEM`

`ProtocolSend.cpp` mostra que o novo sistema é híbrido.

Existem mensagens novas específicas para:

- conexão;
- login;
- character list;
- position;
- movement.

Mas `BOTH_MESSAGE` ainda transporta um pacote clássico e chama `TranslateProtocol()`.

Portanto:

```text
NEW_PROTOCOL_SYSTEM
        |
        +--> mensagens novas específicas
        |
        +--> bridge para protocolo clássico
```

Conclusão: ele não deve ser interpretado como oportunidade para reescrever protocolo junto com o renderer.

Classificação: **PRESERVAR CONTRATO**.

---

## 6. Cliente `Main`: arquitetura real

O projeto Visual Studio já agrupa o cliente em domínios que refletem bem sua arquitetura:

- Map;
- Load;
- Character;
- Skill;
- Guild;
- Sound;
- Buff;
- Effect;
- Text/MultiLanguage;
- Graphic/2D/Vector;
- Common;
- Interface/NewUI;
- Pet;
- Network/proxy/server list;
- GameShop;
- Customs;
- protection/hack/reconnect;
- Lua e bindings BMD/Character/Cloth.

O problema não é ausência de subsistemas; é o elevado acoplamento entre eles.

### 6.1 Bootstrap do cliente

A ordem de inicialização observada em `Winmain.cpp` é sensível:

```text
Protect / MAIN_FILE_INFO
        |
Custom systems
        |
MHP integrated ou plugins
        |
Hack checks
        |
Protocol encryption keys
        |
config.ini / widescreen / language
        |
Window Win32
        |
OpenGL Core Context
        |
CoreGLCompat
        |
Legacy shader bootstrap (OGL330)
        |
In-game updater
        |
Fonts / Input / NewUI / Audio
        |
Scene loop
```

Alterar essa ordem sem necessidade cria risco em UI, proteção, update e gráficos.

### 6.2 Contexto OpenGL atual

`CreateOpenglWindow()` já trabalha em Core Profile:

1. cria contexto bootstrap;
2. procura `wglCreateContextAttribsARB`;
3. tenta 4.6 Core;
4. fallback 4.5;
5. fallback 4.3;
6. fallback 3.3;
7. inicializa GLEW;
8. registra informações de GPU/GLSL;
9. chama `CoreGLCompat::Initialize()`.

Isso confirma que a fundação da modernização está no local correto.

### 6.3 Loop principal

O cliente ainda usa o loop Win32 clássico:

```text
PeekMessage/GetMessage
        |
        +--> Dispatch Win32 messages
        |
        +--> Scene(g_hDC)
        |
        +--> ProtocolCompiler / RecvMessage
```

Quando está no tray, o cliente reduz drasticamente a frequência de `Scene()`.

Importante: processamento de protocolo não é parte do renderer. Não mover protocolo para dentro do RHI/frame graph.

### 6.4 Máquina de estados de cena

`Scene(HDC)` despacha:

```text
MOVIE_SCENE     -> MovieScene
WEBZEN_SCENE    -> WebzenScene
LOADING_SCENE   -> LoadingScene
LOG_IN_SCENE    -> MainScene
CHARACTER_SCENE -> MainScene
MAIN_SCENE      -> MainScene
```

A mesma `MainScene()` serve login, seleção de personagem e gameplay por meio de estados globais.

Isso significa que uma troca global de render path precisa ser testada nos três contextos, não apenas in-game.

---

## 7. Timing e FPS: fronteira gameplay/render

`steady_clock.cpp` é especialmente importante.

Ele controla:

- `FpsLimit`;
- `DeltaT`;
- `speedNormalizer`;
- `visualNormalizer`;
- skill delay;
- ping runtime;
- helpers como `ReduceWithDelta()` e `AdvanceByDelta()`.

Isso é uma fronteira híbrida:

```text
FRAME TIMING
   |
   +--> render cadence
   +--> animation/effect cadence
   +--> skill timing helpers
```

Regra: **não reescrever o timing junto com o RHI**.

O renderer moderno deve primeiro reproduzir o mesmo ritmo observado pelo restante do cliente.

### Inconsistência de configuração encontrada

- `steady_clock` aceita aproximadamente 25–120 FPS;
- o startup dialog em `Winmain.cpp` trabalha até 120 FPS;
- o launcher WPF limita o slider a 64 FPS.

Isso é dívida de configuração, não problema do renderer.

---

## 8. BMD, animação e física

Esta área já recebeu auditoria detalhada nas fases anteriores.

Contratos preservados:

- `Vertex_t::Node`;
- `Normal_t::Node`;
- `BoneTransform[MAX_BONES][3][4]`;
- hierarquia de bones;
- animação/interpolação;
- BMD file format;
- CPU fallback.

O `BoneTransform` CPU **não pode desaparecer** porque ainda é consumido por:

- cloth;
- collision;
- attachment points;
- effects;
- physics;
- outros sistemas dependentes de bones.

GPU Skinning substitui principalmente o custo de transformação **por vértice**, não o sistema de animação CPU.

### Contrato moderno confirmado

```text
Vertex.Node  ------> Bones.x
Normal.Node  ------> Bones.y
BoneIndex    ------> base do skeleton em unidades de bone
BonesTexture ------> skeleton compartilhado pelos passes
```

Formato QPS:

```text
Texel 0 = Quaternion x,y,z,w
Texel 1 = Translation x,y,z + uniform scale
```

Models e Shadows compartilham esse contrato.

Cloth, terrain, effects, interface e fonts possuem pipelines próprios.

---

## 9. Renderer: fronteira segura para modernização

### 9.1 Legado existente

Hoje coexistem:

- chamadas OpenGL/fixed-function históricas;
- `CoreGLCompat` para permitir execução em Core Profile;
- OGL330 legado (`New_ModelBMD`, `u_Bones` etc.);
- infraestrutura moderna `Render/*` em construção.

O OGL330 legado usa palette de 3 `vec4` por bone e `bone * 3`.

Esse contrato foi restaurado e deve continuar intacto enquanto o moderno não estiver validado.

### 9.2 Caminho moderno em construção

O caminho moderno está isolado do runtime e já possui:

- `SkeletonEncoder`;
- `SkeletonBuffer`;
- `OpenGLSkeletonTexture`;
- modern BMD mesh;
- modern VAO;
- modern instance buffer;
- exact shader `GlobalConstants` layout;
- sampler binding separado;
- pose preparation;
- CPU fallback preservado.

Vertex moderno:

```text
location 0 -> Position
location 1 -> UV
location 2 -> Normal
location 3 -> uint2 Bones
```

Instance moderno:

```text
location 6  -> BodyOrigin
location 7  -> BodyScale / normal offset
location 8  -> EnableLight / MinAlpha
location 9  -> UV / shadow metadata
location 10 -> BodyLight
location 11 -> ShadowLight
location 12 -> uint BoneIndex
```

O draw moderno ainda não deve ser ativado globalmente antes de um build/checkpoint e uma validação visual controlada.

### 9.3 Fonte de verdade dos shaders

Arquitetura observada:

```text
HLSL de referência
vulkan-main/vulkan-main/Shaders
        |
        v
geração/cross-compile
        |
        v
GLSL OpenGL
opengl-main/opengl-main/OpenGL
```

Consequência:

- editar HLSL/source de referência quando houver mudança real de shader;
- tratar GLSL como derivado;
- comparar GLSL gerado para validar convenções do backend OpenGL;
- não manter duas lógicas independentes manualmente.

---

## 10. Terrain, effects, cloth, UI e fonts

A auditoria de shader confirma que estes não devem ser forçados para dentro do contrato BMD.

### BMD Models

- GPU skeleton;
- position bone + normal bone;
- instance data;
- BonesTexture.

### Cloth

- não usa `BonesTexture` como BMD models;
- geometria é pré-processada/simulada por outro caminho;
- depende de CPU bones/physics.

### Terrain

- height/light/alpha/attribute textures;
- water/wind/grass;
- arquitetura própria.

### Effects

- instancing;
- procedural quads;
- múltiplos modos no vertex shader.

### Interface

- `Game2D`;
- pipeline independente do 3D BMD.

### Fonts

- glyph quad procedural;
- texture array;
- canal de glyph + outline no mesmo pass.

Regra: modernizar cada domínio **depois** de estabelecer RHI e contracts específicos, não criar um renderer BMD universal.

---

## 11. Lua

### GameServer Lua

`LuaGameServer.cpp` mostra hooks autoritativos para gameplay, incluindo:

- `PlayerCheckDamage`;
- `PlayerAttack`;
- `LevelUpPointAdd`;
- `PlayerMove`;
- `CharacterSet`;
- `EnterCharacter`;
- `PlayerDie`;
- `PlayerLogout`;
- `PlayerDropItem`;
- timers.

O acesso ao Lua é protegido com `gSocketManager.m_critical` em vários caminhos.

Classificação: **NÃO TOCAR** durante modernização gráfica.

### Cliente Lua

O projeto Visual Studio contém domínio `Customs/Lua` e bindings específicos para BMD, Character, Cloth e interfaces.

Isso indica que callbacks Lua podem observar ou dirigir estado visual/gameplay.

Regra: ao migrar funções de render, preservar pontos de callback e ordem relativa onde eles já existem.

---

## 12. Proteção, GetMainInfo e MHP

### 12.1 `MAIN_FILE_INFO`

`GetMainInfo` e `Main/source/Protect.h` definem a mesma estrutura binária.

Ela contém:

- IP/ports;
- version/serial;
- launcher config;
- plugins + CRC;
- Lua config;
- hardware requirements;
- UI options;
- custom options;
- HWID data.

O próprio código registra que os layouts precisam permanecer alinhados byte-for-byte.

`Protect.cpp` valida o tamanho exato do arquivo antes de ler e decodifica a estrutura inteira com XOR.

Classificação: **PRESERVAR CONTRATO BINÁRIO**.

Qualquer inclusão/reordenação de campo exige mudança coordenada no gerador e no cliente.

### 12.2 Plugins

`Protect.cpp`:

1. verifica CRC;
2. `LoadLibrary()`;
3. procura `EntryProc`;
4. executa plugin.

Não relacionar plugin loading com RHI.

### 12.3 MHP integrado

`MHPIntegration.cpp` inclui textualmente dezenas de `.cpp` do MHPClient e também MHPVerify.

Isso produz acoplamento de build incomum:

```text
Main/MHPIntegration.cpp
    |
    +--> #include MHPClient/*.cpp
    +--> #include MHPVerify/*.cpp
```

Classificação: **NÃO TOCAR**, exceto correções mínimas de compilação inevitáveis.

---

## 13. Launcher e atualização

### 13.1 Launcher WPF

Responsabilidades:

- verificar versão local;
- consultar diretório remoto;
- localizar `file_updateNNN.zip`;
- baixar atualização;
- extrair para a pasta do cliente;
- habilitar `Main.exe`;
- editar configurações;
- gravar opções no registro.

### 13.2 Pontos de atenção encontrados

#### Credenciais

O launcher salva `User` e `Password` em `Data/Custom/config.ini` como texto.

Classificação: dívida de segurança/UX fora da Fase 7.

#### Extração do ZIP

A implementação usa `entry.FullName` para construir o destino e extrai diretamente. Não há validação explícita no código auditado garantindo que o caminho canônico final permaneça dentro da pasta do jogo.

Classificação: **potencial risco de path traversal em pacote de update**; merece correção separada.

#### `UseOpenGL`

O launcher pode copiar um `OpenGL32.dll` de `Data/Custom/OpenGL` para a raiz.

Isso é um mecanismo externo/legado e **não é o mesmo sistema** que o Core Profile nativo criado em `Winmain.cpp`.

Regra: não usar essa DLL como fundação do novo RHI.

Ela deve permanecer isolada até que sua função operacional seja decidida separadamente.

---

## 14. MainUpdater e pipeline de pacote

`MainUpdater` é uma ferramenta console C++17.

Ele usa `tar -a -c` para gerar ZIPs.

Modos observados:

```text
Files -> file_updateNNN.zip
Main  -> exe_updateNNN.zip
```

O launcher WPF auditado procura `file_updateNNN.zip`.

Portanto há uma possível divergência de deployment: o modo `exe_updateNNN.zip` do gerador não está consumido pelo fluxo WPF examinado.

Não corrigir isso junto com o renderer; registrar como dívida operacional.

---

## 15. Dados e formatos externos

`MuServer/Data` contém grande parte do comportamento configurável do servidor:

- CashShop;
- character defaults;
- commands;
- custom arenas/events/jewels/mixes/monsters/wings;
- effects;
- BloodCastle;
- ChaosCastle;
- Crywolf;
- DevilSquare;
- DoubleGoer;
- IllusionTemple;
- ImperialGuardian;
- invasion;
- Kanturu;
- Raklion;
- event item bags;
- entre outros.

Esses arquivos constituem contratos operacionais do servidor.

### Regra da modernização

**Não mudar formatos ou interpretação desses arquivos para resolver problemas gráficos do cliente.**

O mesmo vale para:

- BMDs do cliente;
- protocolos;
- SQL schemas/procedures;
- `info.bmd`;
- client version/serial;
- encryption keys.

---

## 16. Banco de dados

DataServer e JoinServer usam ODBC e `QueryManager`.

JoinServer usa diretamente `MEMB_INFO` e procedures como:

- `WZ_GetAccountLevel`;
- `WZ_CONNECT_MEMB`;
- `WZ_DISCONNECT_MEMB`.

DataServer expõe muitos subsistemas sobre o mesmo canal de protocolo para persistência.

Classificação: **NÃO TOCAR**.

Uma modernização de banco, prepared statements, migration layer ou troca de driver seria outro projeto, com testes separados.

---

## 17. Grafo de dependências

O repositório já contém um `graphify-out/GRAPH_REPORT.md`.

Na geração auditada, o relatório registrava aproximadamente:

- 71 mil nós;
- 141 mil arestas;
- mais de 1.700 comunidades.

O grafo foi construído em commit anterior ao HEAD atual, portanto é útil para navegação, mas não deve ser tratado como descrição exata do branch corrente.

Hubs relevantes observados incluem:

- `OBJECTSTRUCT`;
- `BMD`;
- `CreateParticle`;
- `CUIMng`;
- `WSclient`;
- `ZzzOpenglUtil`;
- `PhysicsManager`;
- `Attack`;
- `User`;
- `DataServerProtocol`;
- `CoreGLCompat`;
- `ProtocolSend`;
- `CGMMeshShader`.

Isso confirma que o projeto possui vários "god modules" e pontos de acoplamento elevado.

---

## 18. Dívida técnica observada

### 18.1 God modules

Arquivos como `Winmain.cpp`, `ZzzScene.cpp` e grandes módulos do servidor concentram muitas responsabilidades.

Não realizar uma grande decomposição arquitetural simultânea à migração OpenGL/Vulkan.

### 18.2 Múltiplos render paths

Coexistem:

- fixed-function histórico;
- CoreGLCompat;
- OGL330;
- novo renderer moderno.

Isso é aceitável temporariamente, desde que as fronteiras sejam explícitas e o fallback continue funcional.

### 18.3 Dois mecanismos "OpenGL"

- Core Profile nativo no `Main`;
- opção do launcher que injeta/copia `OpenGL32.dll` externo.

Devem ser tratados como mecanismos distintos.

### 18.4 Configuração duplicada

Existem configurações distribuídas entre:

- `Data/Custom/config.ini`;
- registry `HKCU/Software/MuOnline/Config`;
- `MAIN_FILE_INFO`;
- `MainInfo.ini` no gerador;
- server `.ini/.dat/.txt`.

Modernização futura pode centralizar leitura, mas isso não deve bloquear o renderer.

### 18.5 FPS inconsistente entre ferramentas

Launcher WPF limita de forma diferente do cliente atual.

Resolver depois como tarefa própria.

### 18.6 Updater

Há pontos de hardening e inconsistências de nomenclatura/pacote a tratar fora da trilha gráfica.

---

## 19. Zonas de risco

### NÍVEL CRÍTICO — NÃO TOCAR

```text
GameServer gameplay logic
JoinServer authentication
DataServer persistence
ConnectServer protocol/routing
Packet layouts/encryption
MAIN_FILE_INFO binary layout
BMD file format
MHP integrated protection
SQL schemas/procedures
MuServer gameplay data formats
```

### NÍVEL ALTO — PRESERVAR SEMÂNTICA

```text
BMD animation hierarchy
BoneTransform CPU
SceneFlag state machine
Frame timing / DeltaT
Lua callbacks
UI callbacks
Texture IDs
Render ordering
Shadow semantics
BodyScale / BodyOrigin
```

### NÍVEL ADEQUADO PARA MODERNIZAÇÃO ISOLADA

```text
Render frontend
RHI
OpenGL device/backend
Shader manager
Uniform/UBO binding
Modern BMD VAO
Skeleton buffer / BonesTexture
GPU skinning draw path
Terrain backend após BMD
Effects backend após terrain
UI/font backend após efeitos
Vulkan backend por último
```

---

## 20. Invariantes da branch `modernization`

Durante a modernização:

1. gameplay observado deve permanecer equivalente;
2. protocolo cliente/servidor não muda;
3. GameServer/DataServer/JoinServer/ConnectServer não precisam mudar por causa do renderer;
4. BMD e assets continuam compatíveis;
5. CPU `BoneTransform` permanece disponível;
6. CPU fallback continua funcional;
7. shader HLSL permanece fonte de verdade;
8. OpenGL moderno é validado antes do Vulkan;
9. não remover CoreGLCompat até que todos os consumidores tenham migrado;
10. não ativar globalmente um caminho novo antes de build + teste controlado.

---

## 21. Arquitetura alvo consolidada

```text
                       GAME / MU LOGIC
                              |
        +---------------------+---------------------+
        |                     |                     |
    Network               Gameplay State         Lua/UI
        |                     |                     |
        +---------------------+---------------------+
                              |
                              v
                       RENDER FRONTEND
                              |
          +-------------------+-------------------+
          |                   |                   |
        Models              Terrain            Effects/UI
          |                   |                   |
          +-------------------+-------------------+
                              |
                              v
                             RHI
                          /       \
                         /         \
                    OpenGL        Vulkan
                       |             |
                VALIDAR PRIMEIRO   DEPOIS
```

O frontend deve receber dados renderizáveis, não possuir regras de gameplay.

---

## 22. Sequência recomendada após esta auditoria

### Checkpoint imediato

**BUILD do `Main` agora.**

Motivo: já existe um conjunto suficiente de módulos novos incluídos no MSBuild, mas o draw moderno ainda não foi ligado ao runtime.

É o checkpoint ideal para detectar erros de compilação sem misturá-los com erros visuais/runtime.

### Depois do build limpo

```text
1. Modern shader program loading
2. Bind GlobalConstants
3. Bind Tex1 + BonesTexture
4. Prepare frame SkeletonBuffer
5. Allocate BoneIndex
6. Build BMDModernInstance
7. Draw um único modelo controlado
8. Comparar legado x moderno
9. Testar animação
10. Testar shadow pass
11. Expandir cobertura gradualmente
```

### Somente depois

```text
Terrain
Effects
Water
Shadows avançadas
UI
Fonts
redução de CoreGLCompat
Vulkan
```

---

## 23. Matriz final de decisão

| Área | Ação |
|---|---|
| Client protocol | preservar |
| Server protocol | não tocar |
| GameServer | não tocar |
| JoinServer | não tocar |
| DataServer | não tocar |
| ConnectServer | não tocar |
| DB | não tocar |
| Server Data | preservar |
| MAIN_FILE_INFO | preservar byte-for-byte |
| MHP | não tocar |
| Client Lua hooks | preservar ordem/semântica |
| Server Lua | não tocar |
| BMD format | preservar |
| BoneTransform CPU | preservar |
| CoreGLCompat | manter durante migração |
| OGL330 legacy BMD | manter fallback |
| Modern OpenGL | continuar implementação |
| HLSL shader sources | fonte de verdade |
| GLSL | gerado/validado |
| Vulkan | adiar até RHI estável |
| Launcher/updater | projeto paralelo, não misturar com Fase 7 |

---

## 24. Conclusão

O estudo do projeto completo reforça a estratégia já adotada: **não reescrever o MU; substituir gradualmente o subsistema de renderização atrás de contracts claros**.

O repositório possui forte acoplamento histórico, mas também possui uma fronteira natural: GameServer/JoinServer/DataServer/ConnectServer e os formatos/protocolos podem permanecer inteiramente intocados enquanto o cliente ganha um renderer moderno.

A prioridade técnica deixa de ser "estudar mais shaders indefinidamente" e passa a ser:

```text
entender o próximo contrato
        -> implementar isolado
        -> build
        -> validar contra legado
        -> só então expandir
```

Neste ponto, a auditoria de arquitetura completa sustenta a continuação da Fase 7 sem necessidade de alterar gameplay, protocolo, banco ou servidores.

**Próximo gate recomendado: compilação do `Main` no HEAD atual antes de ativar o Modern BMD Draw.**
