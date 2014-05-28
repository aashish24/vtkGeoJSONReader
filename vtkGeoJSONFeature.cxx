#include "vtkGeoJSONFeature.h"

vtkGeoJSONFeature::vtkGeoJSONFeature()
{
}
vtkPolyData *vtkGeoJSONFeature::GetOutput()
{
    //return the generated polyData after generating the combined polydata from the geoJSON features
    //Check if output data contains appropriate data and return values accordingly
    return outputData;
}
double *vtkGeoJSONFeature::createPoint(Json::Value coordinates)
{
    //Obtain the point in double[] format from its corresponding geoJSON format

    //Check if Coordinates corresponds to Point
    if(!isPoint(coordinates))
    {
        std::cout << "Wrong data format for a point!";
        return NULL;
    }

    //Do isDouble before asDouble to prevent inconsistency
    //Probably check for float/int too

    //Initialise the 3D coordinates to 0
    double *point = new double[3];

    if(coordinates.size() == 1)
    {
        //Update the 3D Coordinates using the 1 Value in the array and rest of the 2 as 0
        Json::Value x = coordinates[0];
        point[0] = x.asDouble();
    }
    else if(coordinates.size() == 2)
    {
        //Update the 3D Coordinates using the 2 Values in the array and 3rd as 0
        Json::Value x = coordinates[0];
        Json::Value y = coordinates[1];
        point[0] = x.asDouble();
        point[1] = y.asDouble();
    }
    else if(coordinates.size() == 3)
    {
        //Update the 3D Coordinates using the 3 Values in the array
        Json::Value x = coordinates[0];
        Json::Value y = coordinates[1];
        Json::Value z = coordinates[2];
        point[0] = x.asDouble();
        point[1] = y.asDouble();
        point[2] = z.asDouble();
    }

    //Return the 3D point as generated above
    return point;
}
vtkPolyData *vtkGeoJSONFeature::extractPoint(Json::Value coordinates, vtkPolyData *outputData)
{
    //Extract the point from geoJSON geometry and generate the corresponding vtkPolyData
    //Check if Coordinates corresponds to Single Point
    if(!isPoint(coordinates))
    {
        std::cout << "Wrong data format for a point!";
        return NULL;
    }

    //Obtain single point data from Json structure to double array
    double *point = createPoint(coordinates);

    //Add a single point to a vtkPolyData
    const int PID_SIZE = 1;
    vtkIdType pid[PID_SIZE];

    vtkPoints *points = outputData->GetPoints();
    pid[0] = points->InsertNextPoint(point);

    vtkCellArray *verts = outputData->GetVerts();
    verts->InsertNextCell(PID_SIZE, pid);

    //Return polydata containing a single point as generated by parsing the data
    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractMultiPoint(Json::Value coordinates, vtkPolyData *outputData)
{
    //Extract multipple points from the geoJSON geometry and create the corresponding vtkPolyData
    //Check if Coordinates corresponds to Multi Points
    if(!isMultiPoint(coordinates))
    {
        std::cout << "Wrong data format for a Multi Point!";
        return NULL;
    }

    if(coordinates.isArray())
    {
        vtkPoints *points = outputData->GetPoints();//Contain the locations of the points
        vtkCellArray *verts = outputData->GetVerts();//Contain the indices corresponding to the position of the vertices

        const int PID_SIZE = coordinates.size();
        vtkIdType pid[PID_SIZE];

        for(int i = 0; i < PID_SIZE; i++)
        {
            //Parse point from Json object to double array and add it to the points array
            double *point = createPoint(coordinates[i]);
            pid[i] = points->InsertNextPoint(point);
        }
        //Update polyData vertices to store multiple points
        verts->InsertNextCell(PID_SIZE, pid);
    }
    //Return polydata containing multiple points as generated above
    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractLineString(Json::Value coordinates, vtkPolyData *outputData)
{
    //extract the line string from the geoJSON geometry and create the corresponding vtkPolyData
    //Check if Coordinates corresponds to Line String
    if(!isLineString(coordinates))
    {
        std::cout << "Wrong data format for a Multi Point!";
        return NULL;
    }

    vtkCellArray *lines = outputData->GetLines();
    vtkPoints *points = outputData->GetPoints();

    int LINE_COUNT = coordinates.size();

    double *start = createPoint(coordinates[0]);
    vtkIdType lineId[2];
    lineId[0] = points->InsertNextPoint(start);

    for(int i = 1; i < LINE_COUNT; i++)
    {
        double *end = createPoint(coordinates[i]);
        lineId[1] = points->InsertNextPoint(end);

        vtkLine *line = vtkLine::New();

        line->GetPointIds()->SetId(0, lineId[0]);
        line->GetPointIds()->SetId(1, lineId[1]);

        lines->InsertNextCell(line);

        start = end;
        lineId[0] = lineId[1];
    }

    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractMultiLineString(Json::Value coordinateArray, vtkPolyData *outputData)
{
    //Extract Multiple Line strings from the geoJSON Geometry and generate the corresponding vtkPolyData
    //Check if Coordinate Array corresponds to Multi Line String
    if(!isMultiLineString(coordinateArray))
    {
        std::cout << "Wrong data format for a Multi Point!";
        return NULL;
    }

    int LINE_STRING_COUNT = coordinateArray.size();

    for(int i = 0; i < LINE_STRING_COUNT; i++)
    {
        extractLineString(coordinateArray[i], outputData);
    }

    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractPolygon(Json::Value coordinate, vtkPolyData *outputData)
{
    //Extract the polygon from the geoJSON feature and generate the corresponding vtkPolyData
    //Check if Coordinate Array corresponds to Polygon
    if(!isPolygon(coordinate))
    {
        std::cout << "Wrong data format for a Multi Point!";
        return NULL;
    }
    bool POLYGON_WITH_HOLES = coordinate.size() > 1 ? true : false;

    vtkPoints *points = outputData->GetPoints();
    vtkCellArray *polys = outputData->GetPolys();

    vtkPolygon *exteriorPoly = vtkPolygon::New();

    //For exterior Polygon
    Json::Value exteriorPolygon = coordinate[0];

    int EXTERIOR_POLYGON_VERTEX_COUNT = exteriorPolygon.size() - 1;
    exteriorPoly->GetPointIds()->SetNumberOfIds(EXTERIOR_POLYGON_VERTEX_COUNT);

    for(int i = 0; i < EXTERIOR_POLYGON_VERTEX_COUNT; i++)
    {
        double *point = createPoint(exteriorPolygon[i]);
        vtkIdType id = points->InsertNextPoint(point);
        exteriorPoly->GetPointIds()->SetId(i, id);
    }
    polys->InsertNextCell(exteriorPoly);

    if(!POLYGON_WITH_HOLES)
        return outputData;

    //Modify polydata to support polygon with holes
    //Probably use delaunay triangulation to create polygon with holes

    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractMultiPolygon(Json::Value coordinateArray, vtkPolyData *outputData)
{
    //Extract the Multiple polygons from the geoJSON geometry and generate the vtkPolyData
    //Check if Coordinate Array corresponds to Multi Polygon
    if(!isMultiPolygon(coordinateArray))
    {
        std::cout << "Wrong data format for a Multi Point!";
        return NULL;
    }

    int POLYGON_COUNT = coordinateArray.size();

    for(int i = 0; i < POLYGON_COUNT; i++)
    {
        //Extract polygon into different polyData and append into a common polyData using the appendPolyData Filter
        extractPolygon(coordinateArray[i], outputData);
    }

    return outputData;
}
vtkPolyData *vtkGeoJSONFeature::extractGeoJSONFeature(Json::Value root, vtkPolyData *outputData)
{
    //Extract the geometry and properties of a geoJSON feature and return the correspongsing vtkPolyData

    this->featureRoot = root;

    Json::Value type = root.get("type", -1);
    Json::Value geometry = root.get("geometry", -1);
    Json::Value properties = root.get("properties", -1);

    vtkPolyData *geometryPolyData = 0;

    if(isEqual(type.asString(), "feature"))
    {
        geometryPolyData = extractGeoJSONFeatureGeometry(geometry, outputData);
    }
    /*
     *Apply geometrical properties from "properties" on the polydata geometry.
    */
    return geometryPolyData;
}
vtkPolyData *vtkGeoJSONFeature::extractGeoJSONFeatureGeometry(Json::Value root, vtkPolyData *outputData)
{
    //Extract Geometry for a geoJSON Feature

    Json::Value type = root.get("type", -1);
    Json::Value coordinates = root.get("coordinates", -1);

    if(type.isString())
    {
        vtkStdString typeString = vtkStdString(type.asString());

        //Extract corresponding geometry into the outputData according to the type value specified in the Json data structures
        if(isEqual(typeString, POINT))
        {
            //Extract point from corresponding geoJSON geometry
            extractPoint(coordinates, outputData);
        }
        else if(isEqual(typeString, MULTI_POINT))
        {
            //Extract multi point from corresponding geoJSON geometry
            extractMultiPoint(coordinates, outputData);
        }
        else if(isEqual(typeString, LINE_STRING))
        {
            //Extract line string from corresponding geoJSON geometry
            extractLineString(coordinates, outputData);
        }
        else if(isEqual(typeString, MULTI_LINE_STRING))
        {
            //Extract multi line string from corresponding geoJSON geometry
            extractMultiLineString(coordinates, outputData);
        }
        else if(isEqual(typeString, POLYGON))
        {
            //Extract polygon from corresponding geoJSON geometry
            extractPolygon(coordinates, outputData);
        }
        else if(isEqual(typeString, MULTI_POLYGON))
        {
            //Extract multi polygon from corresponding geoJSON geometry
            extractMultiPolygon(coordinates, outputData);
        }
        else if(isEqual(typeString, GEOMETRY_COLLECTION))
        {
            std::cout << "This is a Geometry Collection!" << std::endl;
        }
        else
        {
            std::cout << "This geometry ("<< typeString << ") has not been implemented yet";
        }
    }

}
bool vtkGeoJSONFeature::isEqual(vtkStdString str1, vtkStdString str2)
{
    //Case Insensitive string comparison

    //Not matching if string lengths are different
    if(str1.length() != str2.length())
        return false;

    int len = str1.length();
    for(int i = 0; i < len; i++)
    {
        //Exact Match of characters or Capital<->Small match or Small<->Capital match of alphabets
        if(str1[i] != str2[i] && str1[i] - 32 != str2[i] && str1[i] != str2[i] - 32)
            return false;
    }
    return true;
}
bool vtkGeoJSONFeature::isLineString(Json::Value root)
{
    //To Do.
    return true;
}
bool vtkGeoJSONFeature::isMultiLineString(Json::Value root)
{
    //To Do.
    return true;
}
bool vtkGeoJSONFeature::isPoint(Json::Value root)
{
    if(!root.isArray())
    {
        std::cout << "Expected Array as input for point at " << root << std::endl;
        return false;
    }

    if(!(root.size() > 0 && root.size() < 4))
    {
        std::cout << "Expected 3 or less dimension values at " << root << std::endl;
        return false;
    }

    for(int i = 0; i < root.size(); i++)
    {
        Json::Value child = root[i];
        if(!child.isNumeric())
        {
            std::cout <<"Value not Numeric as expected at " << child << std::endl;
            return false;
        }
    }
    return true;
}
bool vtkGeoJSONFeature::isMultiPoint(Json::Value root)
{
    if(!root.isArray())
    {
        std::cout << "Expected Array as input for point at " << root << std::endl;
        return false;
    }
    if(root.size() < 1)
    {
        std::cout << "Expected atleast 1 value at " << root << std::endl;
        return false;
    }
    for(int i = 0; i < root.size(); i++)
    {
        Json::Value child = root[i];
        if(!isPoint(child))
        {
            std::cout << "Child is not a point at " << root;
            return false;
        }
    }
    return true;
}
bool vtkGeoJSONFeature::isPolygon(Json::Value root)
{
    //To Do.
    return true;
}
bool vtkGeoJSONFeature::isMultiPolygon(Json::Value root)
{
    //To Do.
    return true;
}
