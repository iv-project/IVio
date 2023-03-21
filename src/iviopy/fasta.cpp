#include <pybind11/pybind11.h>
#include <ivio/ivio.h>

namespace ivio {

template <typename reader>
struct record_reader {
    reader reader_;

    record_reader(std::string const& path)
        : reader_{{path}}
    {}

    using record_view = typename reader::record_view;
    using record      = typename reader::record;

    static auto init(reader& reader_) {
        return reader_ | std::views::transform([](record_view v) -> record {
            return v;
        });
    }
    decltype(init(reader_)) view = init(reader_);
};
}

namespace py = pybind11;
PYBIND11_MODULE(iviopy, mod) {

    py::module mod_fasta = mod.def_submodule("fasta");

    py::class_<ivio::fasta::record>(mod_fasta, "record")
        .def(py::init<>())
        .def_readwrite("id", &ivio::fasta::record::id)
        .def_readwrite("seq", &ivio::fasta::record::seq)
        .def("__repr__", [](ivio::fasta::record const& o) -> std::string {
            return "<ivio.fasta.record id '" + o.id + "', seq '" + std::string{o.seq} + "'>";
        })
    ;

    py::class_<ivio::record_reader<ivio::fasta::reader>>(mod_fasta, "reader")
        .def(py::init([](std::string path) {
            return std::make_unique<ivio::record_reader<ivio::fasta::reader>>(path);
        }))
        .def("__iter__", [](ivio::record_reader<ivio::fasta::reader>& r) {
            return py::make_iterator(r.view.begin(), r.view.end());
        }, py::keep_alive<0, 1>())
    ;

}
