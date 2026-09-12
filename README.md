# Legend Script

**Legend Script** (ou **Legend**) é uma linguagem de script experimental para jogos, com arquivos `.lgnd`. O projeto começa com uma sintaxe familiar a C/C++ e um runtime pequeno, preparando o caminho para gameplay, entidades, matemática vetorial e integração com engines.

> Este repositório é uma implementação original. O fluxo de build foi inspirado no padrão público do [Terlang](https://github.com/terroo/terlang): CMake, C++23, executável instalável em `bin`, REPL e execução com `-e`.

## Status

Versão atual: **0.1.0 — protótipo da linguagem**.

Já disponível: números, strings, booleanos, `nil`, variáveis (`auto`, `let`, `var`), operadores aritméticos e comparativos, `output`/`out`/`print`, comentários `//`, `sqrt`, REPL e execução de `.lgnd`.

A sintaxe e a ABI ainda podem evoluir. Não é uma engine AAA e não deve ser usada em produção neste estágio.

## Dependências

- Compilador com C++23: GCC, Clang ou MSVC
- CMake 3.25 ou mais recente
- Git

## Compilar e instalar no Linux/macOS/BSD

```bash
git clone https://github.com/beguagua/legend-script.git
cd legend-script
cmake -B build .
cmake --build build
sudo cmake --install build
```

Assim como no Terlang, o comando instalado é `legend`.

```bash
legend --version
legend examples/hello.lgnd
legend -e 'output("Hello from Legend!")'
```

Para instalar em uma pasta local sem `sudo`:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX="$HOME/.local"
cmake --build build
cmake --install build
```

## Windows com MSVC

Abra o Developer PowerShell e execute:

```powershell
git clone https://github.com/beguagua/legend-script.git
cd legend-script
cmake -B build .
cmake --build build --config Release
```

O binário fica em `build\Release\legend.exe`. Adicione essa pasta ao `PATH` ou copie o executável para um diretório já incluído no `PATH`.

## Primeiros passos

```lgnd
auto title = "Legend Script";
auto damage = 12 * 3;
output(title + " damage=" + damage);
output(sqrt(81));
output(damage > 30);
```

Execute:

```bash
legend examples/hello.lgnd
```

O REPL é iniciado sem argumentos. Digite `exit` para sair:

```text
$ legend
Legend Script v0.1.0
legend> output("A new legend begins")
A new legend begins
legend> exit
```

## Direção para jogos AAA

O plano é evoluir por camadas: um lexer/parser com diagnósticos ricos; tipos nativos de jogo (`vec2`, `vec3`, `quat`, `color`, `transform`); entidades e componentes; corrotinas para gameplay; hot reload; bindings para C++/C; compilação AOT e bytecode; e integração opcional com renderizadores e engines. O foco será manter scripts determinísticos, rápidos e seguros para times de gameplay.

## Testes

```bash
cmake -B build -DBUILD_TESTING=ON
cmake --build build
ctest --test-dir build --output-on-failure
```

## Licença

GPL-3.0. Consulte [LICENSE](LICENSE).
