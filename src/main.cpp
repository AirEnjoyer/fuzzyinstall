#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

std::string RunFzf(const char *fzfCommand) {
  std::array<char, 128> buffer;
  std::string result;
  FILE *pipe = popen(fzfCommand, "r");
  if (!pipe) {
    throw std::runtime_error("popen() failed!");
  }
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
    result += buffer.data();
  }
  pclose(pipe);
  return result;
}

int main() {

  std::ifstream osReleaseFile("/etc/os-release");
  if (!osReleaseFile.is_open()) {
    std::cerr << "Error: Could not open /etc/os-release" << std::endl;
    return 1;
  }

  std::string line;
  std::string distro = "Unkown Distro";

  while (std::getline(osReleaseFile, line)) {
    if (line.rfind("ID=", 0) == 0) {
      distro = line.substr(3);
      if (distro.length() >= 2 && distro.front() == '"' &&
          distro.back() == '"') {
        distro = distro.substr(1, distro.length() - 2);
      }
      break;
    }
  }
  osReleaseFile.close();
  std::cout << "Linux Distribution: " << distro << std::endl;

  std::string fzfCommand;
  std::string packageManager = "sudo ";
  if (distro == "Unkown Distro") {
    std::cerr << "Unknow distro" << std::endl;
  } else if (distro == "debian" | distro == "linuxmint" | distro == "ubuntu") {
    fzfCommand = "apt list | awk -F'/' '{print $1}' | grep -v \"Listing...\" | "
                 "sort | fzf";
    packageManager += "apt-get install ";
  } else if (distro == "fedora") {
    fzfCommand = "dnf repoquery --qf \"%{name}\" '*' | fzf";
    packageManager += "dnf install ";
  } else if (distro == "void") {
    fzfCommand = "xbps-query -Rs | fzf ";
    packageManager += "xbps-install -Syu ";
  } else if (distro == "arch") {
    fzfCommand = "pacman -Slq | fzf";
    packageManager += "pacman -Syu ";
  }

  std::string package = packageManager;
  package += RunFzf(fzfCommand.c_str());

  std::system(package.c_str());

  return 0;
}
