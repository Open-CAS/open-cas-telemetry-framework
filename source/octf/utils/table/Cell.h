/*
 * Copyright(c) 2012-2018 Intel Corporation
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOURCE_OCTF_UTILS_TABLE_CELL_H
#define SOURCE_OCTF_UTILS_TABLE_CELL_H

#include <memory>
#include <ostream>
#include <sstream>
#include <string>

namespace octf {
namespace table {

// TODO (trybicki): Provide casting operators
/**
 * @ingroup Table
 *
 * @brief Table cell
 */
class Cell {
public:
    Cell()
            : m_data(new IData()){};
    /**
     * @param other Other Cell
     */
    Cell(const Cell &other)
            : m_data(other.m_data->duplicate()) {}

    /**
     * @brief Assignment operator of other cell
     * @param other Other cell
     * @return Reference to cell
     */
    Cell &operator=(const Cell &other) {
        if (&other == this) {
            return *this;
        }

        m_data = other.m_data->duplicate();
        return *this;
    };

    /**
     * @brief Assignment operator of C string
     * @param value C string value
     * @return Reference to cell
     */
    Cell &operator=(const char *value) {
        *this = std::string(value);
        return *this;
    };

    template <class T>
    bool operator==(const T &value) const {
        std::stringstream osThis, osValue;

        osThis << *this;
        osValue << value;

        return osThis.str() == osValue.str();
    }

    virtual ~Cell() = default;

    /**
     * @brief Generic assignment operator for cell
     * @param value value to be assignment into this cell
     * @return Reference to cell
     */
    template <class T>
    Cell &operator=(const T &value) {
        m_data.reset(new Data<T>(value));

        return *this;
    }

    /**
     * @brief Out stream operator
     * @param os Output stream
     * @param cell Cell to be printed onto output stream
     * @return Reference to stream
     */
    friend std::ostream &operator<<(std::ostream &os, const Cell &cell) {
        cell.m_data->print(os);
        return os;
    }

private:
    class IData {
    public:
        IData() = default;
        virtual ~IData() = default;

        virtual void print(std::ostream &os) const {
            (void) os;
        };

        virtual std::unique_ptr<IData> duplicate() const {
            return std::unique_ptr<IData>(new IData());
        }
    };

    template <class T>
    class Data : public IData {
    public:
        typedef T Type;

        Data(const T &value)
                : IData()
                , m_value(value) {}

        Data(const Data<T> &other)
                : IData()
                , m_value(other.m_value) {}

        virtual void print(std::ostream &os) const override {
            os << m_value;
        };

        virtual std::unique_ptr<IData> duplicate() const override {
            return std::unique_ptr<Data>(new Data<T>(*this));
        }

        virtual ~Data() = default;

    private:
        T m_value;
    };

    std::unique_ptr<IData> m_data;
};

}  // namespace table
}  // namespace octf

#endif  // SOURCE_OCTF_UTILS_TABLE_CELL_H
