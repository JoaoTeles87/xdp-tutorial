# Projeto XDP - Redes de Computadores
**Alunos:** João Teles, [Nome da Dupla]

## Como rodar:

1.  **Instalar pré-requisitos**:
    -   [Vagrant](https://www.vagrantup.com/)
    -   [VirtualBox](https://www.virtualbox.org/)

2.  **Iniciar o Ambiente**:
    Na pasta raiz do projeto, execute:
    ```bash
    vagrant up
    ```

3.  **Acessar o Servidor**:
    ```bash
    vagrant ssh server
    ```

4.  **Compilar o Código**:
    Dentro da VM server:
    ```bash
    cd xdp-tutorial/ping-reply
    # Siga as instruções detalhadas em REPLICATION_GUIDE.md (Seção 3)
    # Envolve copiar para pasta temporária e usar clang manualmente.
    ```

5.  **Carregar o Programa XDP**:
    Ainda na VM server (substitua `eth1` pela interface correta se necessário, ex: `ip a`):
    ```bash
    sudo ip link set dev eth1 xdp obj xdp_prog.o sec xdp
    ```

6.  **Testar (na VM Client)**:
    Em outro terminal:
    ```bash
    vagrant ssh client
    ping 192.168.56.10
    ```
