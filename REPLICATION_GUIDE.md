# Guia de Replicação - Projeto XDP Ping Reply
**Objetivo:** Instruções passo a passo para configurar o ambiente, compilar o projeto e reproduzir os testes de benchmark (Baseline vs XDP).

---

## 1. Pré-requisitos
Para rodar este projeto, você precisa ter instalado na sua máquina host (Windows/Linux/Mac):

1.  **VirtualBox** (Virtualizador)
2.  **Vagrant** (Gerenciador de VMs)
3.  **Git** (Opcional, se for clonar o repositório)

---

## 2. Configuração do Ambiente (Setup)

1.  **Baixe o Projeto**:
    Clone o repositório ou descompacte a pasta do projeto.

2.  **Suba as Máquinas Virtuais**:
    Abra o terminal na pasta raiz do projeto (onde está o `Vagrantfile`) e execute:
    ```bash
    vagrant up
    ```
    *Isso vai baixar a imagem do Ubuntu, criar duas VMs (Server e Client) e instalar os compiladores necessários automaticamente.*

---

## 3. Compilação do Código XDP

Como estamos usando pastas compartilhadas do VirtualBox, precisamos compilar em uma pasta nativa do Linux para evitar erros.

1.  **Acesse a VM Servidor**:
    ```bash
    vagrant ssh server
    ```

2.  **Copie o projeto para uma pasta temporária**:
    ```bash
    # Copia para fora da pasta compartilhada
    cp -r ~/xdp-tutorial ~/xdp-build-temp
    cd ~/xdp-build-temp
    ```

3.  **Prepare as dependências (LibBPF)**:
    ```bash
    ./configure
    cd lib/libbpf/src
    make
    # Instala os headers onde o compilador consegue achar
    mkdir -p ../../../lib/install/include
    make install_headers DESTDIR=../../../lib/install/ OBJDIR=.
    cp -r build/usr/include/bpf ../../../lib/install/include/
    ```

4.  **Compile o Programa XDP**:
    ```bash
    cd ~/xdp-build-temp/ping-reply
    # Compila removendo info de debug (-g) para compatibilidade com o kernel
    clang -O2 -target bpf -c xdp_prog.c -o xdp_prog.o -I../lib/install/include
    ```

5.  **Copie o resultado de volta**:
    ```bash
    # Traz o arquivo compilado para a pasta original do projeto
    cp xdp_prog.o ~/xdp-tutorial/ping-reply/
    ```
    *Agora o arquivo `xdp_prog.o` está pronto na sua pasta original.*

---

## 4. Executando o Benchmark (Teste de Performance)

Para provar que o XDP é mais rápido, faremos dois testes.

### Cenário A: Baseline (Linux Padrão)
*Neste teste, o XDP está deligado. O Kernel do Linux processa o Ping.*

1.  **No Server** (garanta que o XDP está desligado):
    ```bash
    sudo ip link set dev eth1 xdp off
    ```

2.  **No Client** (abra outro terminal e acesse `vagrant ssh client`):
    ```bash
    ping -c 100 192.168.56.10
    ```
    📝 **Anote o tempo médio (`avg`)**. (Esperado: ~1.6ms)

### Cenário B: XDP (Acelerado)
*Neste teste, carregamos o programa XDP para responder direto no driver.*

1.  **No Server** (carregue o programa):
    ```bash
    cd ~/xdp-tutorial/ping-reply
    sudo ip link set dev eth1 xdp obj xdp_prog.o sec xdp
    ```

2.  **No Client** (rode o ping novamente):
    ```bash
    ping -c 100 192.168.56.10
    ```
    📝 **Anote o novo tempo médio**. (Esperado: ~1.0ms - 1.4ms)

---

## 5. Comandos Uteis

- **Verificar se o código está rodando (Logs)**:
    No server: `sudo cat /sys/kernel/debug/tracing/trace_pipe`
- **Remover o programa XDP**:
    No server: `sudo ip link set dev eth1 xdp off`
- **Desligar as VMs (economizar bateria)**:
    No host: `vagrant halt`
- **Destruir o ambiente (limpar disco)**:
    No host: `vagrant destroy -f`
