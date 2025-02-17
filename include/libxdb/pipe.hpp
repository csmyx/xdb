#ifndef XDB_PIPE_HPP
#define XDB_PIPE_HPP

#include <cstddef>
#include <vector>
namespace xdb {
  class pipe {
    public:
      explicit pipe(bool close_on_exec);
      ~pipe();

      int get_read_fd() const { return fds_[read_fd]; }
      int get_write_fd() const { return fds_[write_fd]; }
      int release_read();
      int release_write();
      void close_read();
      void close_write();

      std::vector<std::byte> read();
      void write(std::byte*, std::size_t);

      std::vector<std::byte> finish_read();

    private:
      static constexpr unsigned read_fd = 0;
      static constexpr unsigned write_fd = 1;
      int fds_[2];
  };
}

#endif