#include <array>
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

std::string get_package_manager() {
  std::array<std::string, 6> managers = {"apt-get", "dnf", "xbps-install",
                                         "pacman",  "apk", "zypper"};
  for (const auto &manager : managers) {
    std::string command = "command -v " + manager + " 2>/dev/null";
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
      continue;
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    if (!result.empty()) {
      return manager;
    }
  }
  return "Unknown";
}

int main() {
  std::string pm = get_package_manager();
  if (pm == "Unknown") {
    std::cout << "No package manager found" << std::endl;
  }

  std::string fzfCommand;
  if (pm == "apt-get") {
    fzfCommand = "apt list | awk -F'/' '{print $1}' | grep -v \"Listing...\" | "
                 "sort | fzf";
  } else if (pm == "dnf") {
    fzfCommand = "dnf repoquery --qf \"%{name}\" '*' | fzf";
  } else if (pm == "xbps-install") {
    fzfCommand = "xbps-query -Rs | fzf ";
  }

  std::string package = "sudo " + pm;
  if (pm == "apt-get" | pm == "dnf") {
    package += " install ";
  } else if (pm == "pacman" | pm == "xbps-install") {
    package += " -S ";
  } else if (pm == "apk") {
    package += " add ";
  } else if (pm == "zypper") {
    package += " in ";
  }
  package += RunFzf(fzfCommand.c_str());

  std::cout << package << std::endl;

  return 0;
}
