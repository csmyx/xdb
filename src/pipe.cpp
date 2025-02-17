#include <unistd.h>
#include <fcntl.h>
#include <libxdb/pipe.hpp>
#include <libxdb/error.hpp>
#include <utility>

/// pipe will be closed after exec if close_on_exec is true
xdb::pipe::pipe(bool close_on_exec) {
  if (pipe2(fds_, close_on_exec ? O_CLOEXEC : 0) < 0) {
    error::send_errno("Pipe creation failed");
  }
}

xdb::pipe::~pipe() {
  close_read();
  close_write();
}

int xdb::pipe::release_read() {
  return std::exchange(fds_[read_fd], -1);
}

int xdb::pipe::release_write() {
  return std::exchange(fds_[write_fd], -1);
}

void xdb::pipe::close_read() {
  if (fds_[read_fd] != -1) {
    close(fds_[read_fd]);
    fds_[read_fd] = -1;
  }
}

void xdb::pipe::close_write() {
  if (fds_[write_fd] != -1) {
    close(fds_[write_fd]);
    fds_[write_fd] = -1;
  }
}

std::vector<std::byte> xdb::pipe::read() {
  char buf[1024];
  int chars_read;
  if ((chars_read = ::read(fds_[read_fd], buf, sizeof(buf))) < 0) {
    error::send_errno("Could not read from pipe");
  }
  auto bytes = reinterpret_cast<std::byte*>(buf);
  return std::vector<std::byte>(bytes, bytes + chars_read);
}

void xdb::pipe::write(std::byte* from, std::size_t bytes) {
  if (::write(fds_[write_fd], from, bytes) < 0) {
    error::send_errno("Could not write to pipe");
  }
}

/// Read all bytes from the pipe and close the write end
std::vector<std::byte> xdb::pipe::finish_read() {
  close_write();
  auto bytes = read();
  close_read();
  return bytes;
}


