# Legend Script

**Legend Script** (ou **Legend**) é uma linguagem de gameplay para jogos 2D e 3D. Ela foi desenhada para combinar a facilidade de criação esperada por equipes indie com os recursos de runtime, determinismo e desempenho necessários em produções AAA. Arquivos Legend usam a extensão `.lgnd`.

A ambição do projeto é tornar o código de jogo mais direto que C++, sem abandonar uma base capaz de crescer para compilação nativa, ferramentas profissionais, multiplayer, física, renderização e IA local. A Legend não tenta ser apenas uma camada de scripts: ela está sendo construída como uma plataforma completa de desenvolvimento de jogos.

> O repositório é uma implementação original. O fluxo de build segue o padrão público do [Terlang](https://github.com/terroo/terlang): CMake, C++23, executável instalável em `bin`, REPL e execução com `-e`.

## Status

Versão atual: **0.2.0 — fundação de gameplay 3D**.

Disponível: lexer e parser, números, strings, booleanos, variáveis, operadores, `output`/`out`/`print`, tipos `vec2`, `vec3`, `quat`, `color`, `transform`, entidades, componentes, `spawn`, `add_component`, `set_transform`, `translate`, `entity_count`, corrotinas cooperativas com `yield`, cache de bytecode `.lbc` e hot reload com `--watch`.

O runtime atual é uma fundação executável, não uma engine AAA pronta. Renderização, física e áudio ainda serão módulos nativos da plataforma.

## Instalação e compilação

### Linux, macOS e BSD

```bash
git clone https://github.com/beguagua/legend-script.git
cd legend-script
cmake -B build .
cmake --build build
sudo cmake --install build
```

Para uma instalação local sem `sudo`:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
cmake --install build
```

### Windows com MSVC

```powershell
git clone https://github.com/beguagua/legend-script.git
cd legend-script
cmake -B build .
cmake --build build --config Release
```

O executável ficará em `build\Release\legend.exe`.

## Primeiros passos

Crie `hello.lgnd`:

```lgnd
auto title = "Legend Script";
auto damage = 12 * 3;
output(title + " damage=" + damage);
output(damage > 30);
```

Execute:

```bash
legend hello.lgnd
legend -e 'output("A new legend begins")'
```

Sem argumentos, o comando abre o REPL. Digite `exit` para sair.

## Criando uma cena 3D

Os tipos matemáticos são valores próprios da linguagem. Use coordenadas XYZ diretamente no código para posicionar objetos:

```lgnd
auto player = spawn("Player");
auto position = vec3(0, 1.8, 0);
auto rotation = quat(0, 0, 0, 1);
auto scale = vec3(1, 1, 1);
auto pose = transform(position, rotation, scale);

add_component(player, "CharacterController");
add_component(player, "Health(100)");
set_transform(player, pose);
translate(player, vec3(0, 0, -3));
output(player);
```

### Tipos principais

| Tipo | Uso |
|---|---|
| `vec2(x, y)` | Coordenadas 2D, UI e input |
| `vec3(x, y, z)` | Posição, direção, escala e velocidade 3D |
| `quat(x, y, z, w)` | Rotação sem gimbal lock |
| `color(r, g, b, a)` | Cor linear/RGBA |
| `transform(position, rotation, scale)` | Pose completa de um objeto |
| `spawn("Name")` | Cria uma entidade no runtime |
| `add_component(entity, "Type")` | Anexa um componente ao objeto |

## Tutorial: primeiro protótipo de FPS

O arquivo [`examples/fps_foundations.lgnd`](examples/fps_foundations.lgnd) demonstra a base de um FPS. O fluxo recomendado é:

1. Crie o jogador com `spawn("Player")`.
2. Adicione componentes de movimento e vida.
3. Crie a câmera e a arma como entidades separadas.
4. Use `vec3` para posicionar cada objeto em XYZ.
5. Use `transform` para combinar posição, rotação e escala.
6. Use `translate` para simular o movimento do jogador.
7. Use `yield(0.016)` para entregar o controle ao scheduler no fim do frame.

```bash
legend examples/fps_foundations.lgnd
```

O próximo módulo do FPS adicionará input, colisão, raycast, meshes, materiais, animação e renderização. A linguagem manterá essas APIs de alto nível, enquanto o runtime poderá usar backends Vulkan, DirectX ou Metal.

## Corrotinas

Corrotinas permitem scripts de gameplay suspenderem sua execução sem bloquear o jogo:

```lgnd
output("opening door");
yield(0.25);
output("door opened");
```

O `yield` atual registra o ponto de espera no scheduler cooperativo. A futura versão do runtime usará esse mesmo contrato para sequências de animação, diálogos, streaming e IA.

## Bytecode e hot reload

Compile um script para o cache binário Legend:

```bash
legend --compile examples/fps_foundations.lgnd
legend --bytecode examples/fps_foundations.lbc
```

O bytecode atual é um formato seguro e simples de cache do programa-fonte, criado para estabilizar a interface do pipeline. A próxima etapa substituirá o conteúdo pelo instruction set compacto e verificável da VM.

Durante o desenvolvimento, observe um arquivo e recarregue-o quando ele mudar:

```bash
legend --watch examples/fps_foundations.lgnd
```

## IA local e arquivos GGUF

A arquitetura prevê um módulo opcional `legend-ai` para NPCs inteligentes. Ele poderá carregar um arquivo `.gguf`, registrar um componente de agente e conversar com um backend baseado em `llama.cpp`. Essa integração será opt-in porque modelos GGUF são grandes e variam em licença, memória e hardware:

```lgnd
// API planejada — ainda não habilitada no runtime 0.2
auto npc = spawn("Guard");
add_component(npc, "DialogueAgent(model=guard.gguf)");
```

O projeto não baixa modelos nem compila `llama.cpp` automaticamente nesta versão. Isso evita downloads inesperados e permite que cada equipe escolha modelo, licença, quantização e backend.

## Testes

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Roadmap

As próximas camadas são: VM de bytecode real e debugger; sistema de tipos estático opcional; renderização 3D e materiais; física e navegação; input e networking; hot reload de assets; bindings para Vulkan/DirectX/Metal; compilação AOT; editor visual; e módulo opcional de IA GGUF/llama.cpp.

O objetivo é que iniciantes possam criar protótipos rapidamente e que equipes profissionais possam escalar o mesmo projeto sem trocar de linguagem.

## Licença

GPL-3.0. Consulte [LICENSE](LICENSE).


## Runtime de jogo 0.3

A versão 0.3 adiciona uma camada de runtime para construir um vertical slice jogável sem sair da Legend. Ela inclui cenas, entidades, componentes, input abstrato, colisão espacial básica, áudio, desenho de meshes, relógio de jogo e controle do ciclo de execução.

```lgnd
scene("arena");
auto player = spawn("Player");
auto enemy = spawn("Enemy");
add_component(player, "CharacterController");
add_component(enemy, "NavAgent");
set_transform(player, transform(vec3(0, 1.8, 0), quat(0,0,0,1), vec3(1,1,1)));
translate(player, vec3(input_axis("move_x") * delta_time(), 0, -delta_time()));
draw_mesh("player.mesh", player, "player.mat");
play_sound("music/arena.ogg");
```

APIs disponíveis nesta versão: `scene`, `current_scene`, `time`, `delta_time`, `input_pressed`, `input_axis`, `play_sound`, `draw_mesh`, `collides`, `log` e `quit`. Os backends reais de Vulkan/DirectX/Metal, áudio e input serão conectados ao mesmo contrato em módulos nativos posteriores; o runtime atual oferece uma implementação verificável e substituível para desenvolvimento da linguagem.

Execute o exemplo completo:

```bash
legend --run examples/complete_game.lgnd
```

O objetivo da Legend é permitir que o mesmo código seja usado para protótipos indie, ferramentas internas e jogos AAA. Para isso, o projeto está sendo separado em uma linguagem expressiva, uma VM de bytecode, um runtime de entidades e componentes e backends de plataforma. A versão 0.3 é o primeiro vertical slice dessa arquitetura, não uma promessa de que renderização, física e networking de produção já estejam prontos.


## Backend OpenGL

A Legend oferece um contrato de backend OpenGL para todas as versões desktop de **OpenGL 1.0 até OpenGL 4.6**. O runtime valida a versão solicitada e diferencia o perfil de compatibilidade do perfil core:

```bash
legend --opengl-version 1.0
legend --opengl-version 2.1
legend --opengl-version 3.3
legend --opengl-version 4.6 core
```

O resultado informa a capacidade pedida, por exemplo `OpenGL 4.6 Core (core)`. OpenGL core exige 3.2 ou superior; versões antigas usam o perfil de compatibilidade. A criação real do contexto é responsabilidade do host de janela de cada plataforma — WGL no Windows, GLX/EGL no Linux e CGL no macOS — e o driver instalado ainda precisa oferecer a versão solicitada. A Legend não pode inventar suporte de hardware que o sistema não possui.

O CMake tenta localizar o SDK OpenGL quando disponível:

```bash
cmake -B build -DLEGEND_ENABLE_OPENGL=ON .
```

Se o SDK não estiver instalado, a camada de negociação portátil continua compilando e os jogos podem selecionar um backend nativo posteriormente. Para builds sem qualquer integração gráfica:

```bash
cmake -B build -DLEGEND_ENABLE_OPENGL=OFF .
```

A API de alto nível de `draw_mesh`, materiais e cenas permanece independente do backend. Isso permite que o mesmo jogo use OpenGL 1.x/2.x em hardware antigo, OpenGL 3.3 para uma base ampla ou OpenGL 4.6 Core para recursos modernos.


## Vídeo: gameplay em CPU/RAM

Como o ambiente de demonstração não possui placa de vídeo disponível, executei o vertical slice `examples/complete_game.lgnd` em modo headless: o runtime processou entidades, cena, colisão, input, áudio e renderização abstrata em memória RAM, enquanto uma visualização 2D foi gravada para demonstrar o estado do jogo.

[Baixar vídeo da execução em CPU/RAM](media/legend-cpu-ram-gameplay.mp4)

Esse modo é útil para testes automatizados, servidores e CI. Não substitui um backend gráfico real para jogos AAA; em uma máquina com GPU, a mesma camada de gameplay poderá ser conectada ao OpenGL, Vulkan, DirectX ou Metal.
