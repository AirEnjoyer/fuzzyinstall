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

int main(int argc, char *argv[]) {

  std::string line;
  std::string distro = "Unkown distro";
  std::string distroLike = "Unknown base distro";
  std::string fzfCommand;
  std::string packageManager = "sudo ";
  if (argc > 1) {
    std::string arg1 = argv[1];
    if (arg1 == "apt") {
      distro = "debian";
    } else if (arg1 == "dnf") {
      distro = "fedora";
    } else if (arg1 == "xbps") {
      distro = "void";
    } else if (arg1 == "pacman") {
      distro = "arch";
    } else {
      std::cout << "Unknown package manager: " << arg1 << std::endl
                << "Options:"
                << "apt, dnf, xbps, pacman" << std::endl;
    }
  }

  std::ifstream osReleaseFile("/etc/os-release");
  if (!osReleaseFile.is_open()) {
    std::cerr << "Error: Could not open /etc/os-release" << std::endl;
    return 1;
  }

  while (std::getline(osReleaseFile, line)) {
    if (line.rfind("ID=", 0) == 0) {
      distro = line.substr(3);
      if (distro.length() >= 2 && distro.front() == '"' &&
          distro.back() == '"') {
        distro = distro.substr(1, distro.length() - 2);
      }
    }

    if (line.rfind("ID_LIKE=", 0) == 0) {
      distroLike = line.substr(8);
      if (distroLike.length() >= 2 && distroLike.front() == '"' &&
          distro.back() == '"') {
        distro = distroLike.substr(1, distroLike.length() - 2);
      }
    }
  }
  osReleaseFile.close();

  if (distro == "Unkown distro" && distroLike == "Unknow base distro") {
    std::cerr << "Unknow distro" << std::endl;
    return 1;
  } else if (distro == "debian" | distroLike == "ubuntu" |
             distroLike == "debian") {
    fzfCommand = "apt list | awk -F'/' '{print $1}' | grep -v \"Listing...\" | "
                 "sort | fzf";
    packageManager += "apt-get install ";
  } else if (distro == "fedora" | distroLike == "rhel fedora" |
             distroLike == "fedora") {
    fzfCommand = "dnf repoquery --qf \"%{name}\" '*' | fzf";
    packageManager += "dnf install ";
  } else if (distro == "void") {
    fzfCommand = "xbps-query -Rs | fzf ";
    packageManager += "xbps-install -Syu ";
  } else if (distro == "arch" | distroLike == "arch") {
    fzfCommand = "pacman -Slq | fzf";
    packageManager += "pacman -Syu ";
  }

  std::string package = packageManager;
  package += RunFzf(fzfCommand.c_str());

  std::system(package.c_str());

  return 0;
}
