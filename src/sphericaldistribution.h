#ifndef SPHERICALDISTRIBUTION_H
#define SPHERICALDISTRIBUTION_H

#include <random>
#include <cmath>

const float PI(3.14159);

template <typename T>
class SphericalDistribution
{
    using result_type = T;
    using param_type = void;

public:
    SphericalDistribution() = default;
    SphericalDistribution(const SphericalDistribution &) = default;
    SphericalDistribution & operator=(const SphericalDistribution &) = default;

    void reset() { } //why the fuck i need that ?
    void param() { }
    result_type min() const { return 0; }
    result_type max() const { return PI; }

    template <typename U> //idk why i need to use an other type, it's the same
    friend std::ostream& operator<<(std::ostream& os, const SphericalDistribution<U>& d);
    template <typename U>
    friend std::istream& operator>>(std::istream& os, SphericalDistribution<U>& d);
    bool operator==(const SphericalDistribution<T> & d) const { return true; }
    bool operator!=(const SphericalDistribution<T> & d) const { return !(d==*this); }

    template <typename GEN>
    result_type operator() (GEN & generator)
    {
        result_type value(2*(generator()-generator.min())/((result_type)(generator.max()-generator.min()))-1);
        return acos(value);
    }

private:
};


template <typename T>
std::ostream& operator<<(std::ostream& out, const SphericalDistribution<T>& d)
{
    //out << d.m_radius << std::endl;
    return out;
}

template <typename T>
std::istream& operator>>(std::istream& in, SphericalDistribution<T>& d)
{
    //in >> d.m_radius;
    return in;
}

#endif // SPHERICALDISTRIBUTION_H
