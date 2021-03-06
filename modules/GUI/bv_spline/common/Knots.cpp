
#include "Interpolation.h"

namespace bav::spline
{
Knot::Knot (float xToUse, float yToUse)
    : location (xToUse, yToUse)
{
}

Knot& Knot::operator= (const Knot& other)
{
    location  = other.location;
    dragStart = other.dragStart;
    selected  = other.selected;

    return *this;
}

void Knot::serialize (TreeReflector& ref)
{
    ref.add ("Location", location);
    ref.add ("DragStart", dragStart);
    ref.add ("Selected", selected);
}

void Knot::select() noexcept
{
    dragStart = location;
    selected  = true;
}

void Knot::deselect()
{
    selected = false;
}

bool Knot::isSelected() const
{
    return selected;
}

bool Knot::drag (const Point& p) noexcept
{
    if (! selected) return false;

    location.x = dragStart.x + p.x;
    location.y = dragStart.y + p.y;

    return true;
}

Point Knot::getDenormalizedPoint (const juce::Rectangle< float >& bounds) const
{
    return {
        bounds.getX() + bounds.getWidth() * location.x,
        bounds.getY() + bounds.getHeight() * location.y};
}


Knots::Knots()
{
    add ({0, 0});
    add ({1, 1});
}

const Knot& Knots::getKnot (int index) const
{
    jassert (index >= 0 && index < static_cast< int > (size()));
    return (*this)[static_cast< size_type > (index)];
}

void Knots::serialize (TreeReflector& ref)
{
    ref.add ("Knot", *(static_cast< std::vector< Knot >* > (this)));
}

void Knots::add (const Point& p)
{
    push_back ({p.x, p.y});
}

void Knots::sort()
{
    struct Sorter
    {
        bool operator() (const Knot& a, const Knot& b) const noexcept { return a.location.x < b.location.x; }
    };

    std::sort (begin(), end(), Sorter());
}

void Knots::removeOffLimits()
{
    struct Remover
    {
        bool operator() (const Knot& knot) const noexcept
        {
            if (! knot.isSelected()) return false;
            return knot.location.x <= 0.f || knot.location.x >= 1.f || knot.location.y <= 0.f || knot.location.y >= 1.f;
        }
    };

    std::remove_if (begin(), end(), Remover());
}

void Knots::remove (const juce::Range< float >& range)
{
    for (auto knot = begin(); knot != end(); ++knot)
    {
        if (range.getEnd() < knot->location.x) break;

        if (range.getStart() < knot->location.x)
            erase (knot);
    }
}

void Knots::select (const juce::Range< float >& range) noexcept
{
    for (auto& knot : *this)
    {
        if (knot.location.x > range.getEnd()) return;

        if (knot.location.x >= range.getStart())
            knot.select();
    }
}

bool Knots::drag (const Point& drag) noexcept
{
    bool changed = false;

    for (auto& knot : *this)
        if (knot.drag (drag))
            changed = true;

    return changed;
}

void Knots::deselect()
{
    for (auto& knot : *this)
        knot.deselect();
}

void Knots::makeSpline (Points& spline) const
{
    auto       x                = 0.f;
    const auto inc              = 1.f / static_cast< float > (spline.size());
    const auto smallestDistance = inc * 2.f;
    int        kIdx             = 1;

    for (size_t i = 0; i < spline.size();
         ++i, x += inc)
    {
        if (x >= getKnot (kIdx).location.x)
        {
            ++kIdx;
            kIdx %= size();
        }

        spline[i] = juce::jlimit (0.f, 1.f,
                                  interpolation::hermitCubic2 (*this, x, smallestDistance, kIdx - 1));
    }
}


void Spline::serialize (TreeReflector& ref)
{
    ref.add ("Knots", knots);
    ref.add ("Point", points);
}

void Spline::updatePoints (const juce::Rectangle< float >& bounds)
{
    knots.makeSpline (points);

    const auto height = bounds.getHeight();
    const auto y      = bounds.getY();

    for (auto& point : points)
        point = point * height + y;
}

void Spline::resize (int newNumPoints)
{
    points.resize (static_cast< Points::size_type > (newNumPoints));
}

float Spline::getPoint (int index) const
{
    return points[static_cast< Points::size_type > (index)];
}

}  // namespace bav::spline
