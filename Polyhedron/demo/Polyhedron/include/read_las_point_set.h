#ifndef READ_LAS_POINT_SET_H
#define READ_LAS_POINT_SET_H

#include <liblas/liblas.hpp>

template <typename Point, typename Vector>
bool read_las_point_set(
  std::istream& stream, ///< input stream.
  CGAL::Point_set_3<Point, Vector>& point_set) ///< point set
{
  typename CGAL::Point_set_3<Point, Vector> Point_set;
  
  liblas::ReaderFactory f;
  liblas::Reader reader = f.CreateWithStream(stream);

  Point_set::Property_map<unsigned char> echo_map;
  boost::tie(echo_map, boost::tuples::ignore)
    = point_set.template add_property_map<unsigned char>("echo", 0);
  
  Point_set::Property_map<unsigned char> red_map;
  boost::tie(red_map, boost::tuples::ignore)
    = point_set.template add_property_map<unsigned char>("red", 0);
  Point_set::Property_map<unsigned char> green_map;
  boost::tie(green_map, boost::tuples::ignore)
    = point_set.template add_property_map<unsigned char>("green", 0);
  Point_set::Property_map<unsigned char> blue_map;
  boost::tie(blue_map, boost::tuples::ignore)
    = point_set.template add_property_map<unsigned char>("blue", 0);

  while (reader.ReadNextPoint())
    {
      const liblas::Point& p = reader.GetPoint();
      Point_set::iterator it = point_set.insert (Kernel::Point_3 (p.GetX(), p.GetY(), p.GetZ()));
      put(echo_map, *it, p.GetReturnNumber());
      const liblas::Color& c = p.GetColor();
      put(red_map, *it, (c.GetRed() >> 8));
      put(green_map, *it, (c.GetGreen() >> 8));
      put(blue_map, *it, (c.GetBlue() >> 8));
    }

  bool remove_echo = true;
  bool remove_colors = true;
  
  for (Point_set::iterator it = point_set.begin(); it != point_set.end(); ++ it)
    {
      if (get(echo_map, *it) != 0)
        remove_echo = false;
      if (get(red_map, *it) != 0 ||
          get(green_map, *it) != 0 ||
          get(blue_map, *it) != 0)
        remove_colors = false;
      if (!remove_echo && !remove_colors)
        break;
    }

  if (remove_echo)
    point_set.remove_property_map(echo_map);
  if (remove_colors)
    {
      point_set.remove_property_map(red_map);
      point_set.remove_property_map(green_map);
      point_set.remove_property_map(blue_map);
    }
  
  return true;
}


#endif // READ_LAS_POINT_SET_H
