#include <gtest/gtest.h>
#include <iostream>
#include "arrow/api.h"
#include "arrow/array.h"
#include "gis/cpu/geometry.h"
#include<ctime>

using namespace zilliz::gis;

TEST(geometry_test, make_point_from_double){

  arrow::DoubleBuilder builder_x;
    arrow::DoubleBuilder builder_y;
    std::shared_ptr<arrow::Array> ptr_x;
    std::shared_ptr<arrow::Array> ptr_y;

    for (int i = 0; i < 2; i++) {
        builder_x.Append(i);
        builder_y.Append(i);
    }

    builder_x.Finish(&ptr_x);
    builder_y.Finish(&ptr_y);

    auto point_arr = ST_Point(ptr_x,ptr_y);
    auto point_arr_str = std::static_pointer_cast<arrow::StringArray>(point_arr);

    ASSERT_EQ(point_arr_str->length(),2);
    ASSERT_EQ(point_arr_str->GetString(0),"POINT (0 0)");
    ASSERT_EQ(point_arr_str->GetString(1),"POINT (1 1)");
}


TEST(geometry_test,test_ST_IsValid){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);


  OGRLinearRing ring3;
  ring3.addPoint(2, 1);
  ring3.addPoint(3, 1);
  ring3.addPoint(3, 2);
  ring3.addPoint(2, 2);
  ring3.addPoint(2, 8);
  ring3.closeRings();
  OGRPolygon polygon3;
  polygon3.addRing(&ring3);

  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> polygons;
  
  string_builder.Append(polygon1.exportToWkt());
  string_builder.Append(polygon2.exportToWkt());
  string_builder.Append(polygon3.exportToWkt());

  string_builder.Finish(&polygons);

  auto vaild_mark = ST_IsValid(polygons);
  auto vaild_mark_arr = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);

  ASSERT_EQ(vaild_mark_arr->Value(0),true);
  ASSERT_EQ(vaild_mark_arr->Value(1),true);
  ASSERT_EQ(vaild_mark_arr->Value(2),false);
}

TEST(geometry_test, test_ST_Intersection){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);

  arrow::StringBuilder left_string_builder;
  std::shared_ptr<arrow::Array> left_polygons;
  arrow::StringBuilder right_string_builder;
  std::shared_ptr<arrow::Array> right_polygons;

  left_string_builder.Append(polygon1.exportToWkt());
  right_string_builder.Append(polygon2.exportToWkt());

  left_string_builder.Finish(&left_polygons);
  right_string_builder.Finish(&right_polygons);

  auto intersection_geometries = ST_Intersection(left_polygons,right_polygons);
  auto intersection_geometries_arr = std::static_pointer_cast<arrow::StringArray>(intersection_geometries);
  
  ASSERT_EQ(intersection_geometries_arr->GetString(0),"LINESTRING (2 2,2 1)");
}

TEST(geometry_test, test_ST_PrecisionReduce){
  OGRPoint point(1.5555555,1.55555555);
  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> array;
  string_builder.Append(point.exportToWkt());
  string_builder.Finish(&array);
  auto geometries = ST_PrecisionReduce(array,6);
  auto geometries_arr = std::static_pointer_cast<arrow::StringArray>(geometries);
  
  ASSERT_EQ(geometries_arr->GetString(0),"POINT (1.55556 1.55556)");
}

TEST(geometry_test, test_ST_Equals){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);

  arrow::StringBuilder left_string_builder;
  arrow::StringBuilder right_string_builder;
  std::shared_ptr<arrow::Array> left_polygons;
  std::shared_ptr<arrow::Array> right_polygons;

  left_string_builder.Append(polygon1.exportToWkt());
  left_string_builder.Append(polygon2.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());

  left_string_builder.Finish(&left_polygons);
  right_string_builder.Finish(&right_polygons);

  auto vaild_mark = ST_Equals(left_polygons,right_polygons);
  auto vaild_mark_str = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);

  ASSERT_EQ(vaild_mark_str->Value(0),true);
  ASSERT_EQ(vaild_mark_str->Value(1),false);
}

TEST(geometry_test, test_ST_Touches){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);

  arrow::StringBuilder left_string_builder;
  arrow::StringBuilder right_string_builder;
  std::shared_ptr<arrow::Array> left_polygons;
  std::shared_ptr<arrow::Array> right_polygons;

  left_string_builder.Append(polygon1.exportToWkt());
  left_string_builder.Append(polygon2.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());

  left_string_builder.Finish(&left_polygons);
  right_string_builder.Finish(&right_polygons);

  auto vaild_mark = ST_Touches(left_polygons,right_polygons);
  auto vaild_mark_str = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);
   
  ASSERT_EQ(vaild_mark_str->Value(0),false);
  ASSERT_EQ(vaild_mark_str->Value(1),true);
}

TEST(geometry_test, test_ST_Overlaps){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);

  arrow::StringBuilder left_string_builder;
  arrow::StringBuilder right_string_builder;
  std::shared_ptr<arrow::Array> left_polygons;
  std::shared_ptr<arrow::Array> right_polygons;

  left_string_builder.Append(polygon1.exportToWkt());
  left_string_builder.Append(polygon2.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());

  left_string_builder.Finish(&left_polygons);
  right_string_builder.Finish(&right_polygons);

  auto vaild_mark = ST_Overlaps(left_polygons,right_polygons);
  auto vaild_mark_str = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);
   
  ASSERT_EQ(vaild_mark_str->Value(0),false);
  ASSERT_EQ(vaild_mark_str->Value(1),false);
}


TEST(geometry_test, test_ST_Crosses){
  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);

  arrow::StringBuilder left_string_builder;
  arrow::StringBuilder right_string_builder;
  std::shared_ptr<arrow::Array> left_polygons;
  std::shared_ptr<arrow::Array> right_polygons;

  left_string_builder.Append(polygon1.exportToWkt());
  left_string_builder.Append(polygon2.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());
  right_string_builder.Append(polygon1.exportToWkt());

  left_string_builder.Finish(&left_polygons);
  right_string_builder.Finish(&right_polygons);

  auto vaild_mark = ST_Crosses(left_polygons,right_polygons);
  auto vaild_mark_str = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);
   
  ASSERT_EQ(vaild_mark_str->Value(0),false);
  ASSERT_EQ(vaild_mark_str->Value(1),false);
}


TEST(geometry_test, test_ST_IsSimple){

  OGRLinearRing ring1;
  ring1.addPoint(1, 1);
  ring1.addPoint(1, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 1);
  ring1.addPoint(1, 1);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRLinearRing ring2;
  ring2.addPoint(2, 1);
  ring2.addPoint(3, 1);
  ring2.addPoint(3, 2);
  ring2.addPoint(2, 2);
  ring2.addPoint(2, 1);
  ring2.closeRings();
  OGRPolygon polygon2;
  polygon2.addRing(&ring2);


  OGRLinearRing ring3;
  ring3.addPoint(2, 1);
  ring3.addPoint(3, 1);
  ring3.addPoint(3, 2);
  ring3.addPoint(2, 2);
  ring3.addPoint(2, 8);
  ring3.closeRings();
  OGRPolygon polygon3;
  polygon3.addRing(&ring3);

  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> polygons;
  
  string_builder.Append(polygon1.exportToWkt());
  string_builder.Append(polygon2.exportToWkt());
  string_builder.Append(polygon3.exportToWkt());

  string_builder.Finish(&polygons);

  auto vaild_mark = ST_IsSimple(polygons);
  auto vaild_mark_arr = std::static_pointer_cast<arrow::BooleanArray>(vaild_mark);

  ASSERT_EQ(vaild_mark_arr->Value(0),true);
  ASSERT_EQ(vaild_mark_arr->Value(1),true);
  ASSERT_EQ(vaild_mark_arr->Value(2),false);
}

TEST(geometry_test, test_ST_MakeValid){
  OGRLinearRing ring1;
  ring1.addPoint(2, 1);
  ring1.addPoint(3, 1);
  ring1.addPoint(3, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 8);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> polygons;

  string_builder.Append(polygon1.exportToWkt());
  string_builder.Finish(&polygons);
  
  auto geometries = ST_MakeValid(polygons);
  auto geometries_arr = std::static_pointer_cast<arrow::StringArray>(geometries);
  
  ASSERT_EQ(geometries_arr->GetString(0),"GEOMETRYCOLLECTION (POLYGON ((2 2,3 2,3 1,2 1,2 2)),LINESTRING (2 2,2 8))");
}

TEST(geometry_test, test_ST_GeometryType){
  OGRLinearRing ring1;
  ring1.addPoint(2, 1);
  ring1.addPoint(3, 1);
  ring1.addPoint(3, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 8);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRPoint point(2,3);

  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> geometries;

  string_builder.Append(polygon1.exportToWkt());
  string_builder.Append(point.exportToWkt());
  string_builder.Finish(&geometries);

  auto geometries_type = ST_GeometryType(geometries);
  auto geometries_type_arr = std::static_pointer_cast<arrow::StringArray>(geometries_type);
  
  ASSERT_EQ(geometries_type_arr->GetString(0),"POLYGON");
  ASSERT_EQ(geometries_type_arr->GetString(1),"POINT");
}

TEST(geometry_test, test_ST_SimplifyPreserveTopology){
  OGRLinearRing ring1;
  ring1.addPoint(2, 1);
  ring1.addPoint(3, 1);
  ring1.addPoint(3, 2);
  ring1.addPoint(2, 2);
  ring1.addPoint(2, 8);
  ring1.closeRings();
  OGRPolygon polygon1;
  polygon1.addRing(&ring1);

  OGRPoint point(2,3);

  arrow::StringBuilder string_builder;
  std::shared_ptr<arrow::Array> geometries;

  string_builder.Append(polygon1.exportToWkt());
  string_builder.Append(point.exportToWkt());
  string_builder.Finish(&geometries);

  auto geometries_arr = ST_SimplifyPreserveTopology(geometries,10000);
  auto geometries_arr_str = std::static_pointer_cast<arrow::StringArray>(geometries_arr);
  
  ASSERT_EQ(geometries_arr_str->GetString(0),"POLYGON ((2 1,3 1,2 8,2 1))");
  ASSERT_EQ(geometries_arr_str->GetString(1),"POINT (2 3)");
}