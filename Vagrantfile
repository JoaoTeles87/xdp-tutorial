# -*- mode: ruby -*-
# vi: set ft=ruby :

Vagrant.configure("2") do |config|
  # Use the same box for both
  config.vm.box = "generic/ubuntu2004"
  
  # Sync folder: Your Windows folder maps to /home/vagrant/xdp-tutorial
  config.vm.synced_folder ".", "/home/vagrant/xdp-tutorial"

  # Common Setup Script
  $script = <<-SHELL
    apt-get update
    apt-get install -y clang-12 llvm-12 libelf-dev libpcap-dev gcc-multilib build-essential linux-tools-generic m4
    # Set alternatives to ensure clang-12 is the default 'clang'
    update-alternatives --install /usr/bin/clang clang /usr/bin/clang-12 100
    update-alternatives --install /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-12 100
    apt-get install -y linux-headers-$(uname -r) pkg-config tcpdump
    # Fix for bpftool path if needed
    ln -s /usr/lib/linux-tools/$(uname -r)/bpftool /usr/sbin/bpftool 2>/dev/null || true
  SHELL

  # --- SERVER VM (Runs XDP) ---
  config.vm.define "server" do |server|
    server.vm.hostname = "xdp-server"
    server.vm.network "private_network", ip: "192.168.56.10"
    server.vm.provider "virtualbox" do |vb|
      vb.memory = "2048"
      vb.cpus = 2
    end
    server.vm.provision "shell", inline: $script
  end

  # --- CLIENT VM (Sends Ping) ---
  config.vm.define "client" do |client|
    client.vm.hostname = "xdp-client"
    client.vm.network "private_network", ip: "192.168.56.11"
    client.vm.provider "virtualbox" do |vb|
      vb.memory = "1024"
      vb.cpus = 1
    end
    client.vm.provision "shell", inline: $script
  end
end