import json
import sys


#
# swagger model generator for cpp.
#
# given the file swagger.json in the current directory, generate a 
# single include file containing model definitions
#

def caseize_class(class_name):
    class_name = class_name.replace('-', '_')
    class_name = class_name[0].upper() + class_name[1:]
    return class_name


def caseize_property(property):
    property = property.replace('-', '_')
    property = property[0].lower() + property[1:]
    return property


def get_property_type(property_dict):
    if 'type' in property_dict:
        spec_type = property_dict['type']

        if spec_type == 'string':
            return 'std::string'

        if spec_type == 'number' and property_dict['format'] == 'float':
            return 'float'

        if spec_type == 'number' or spec_type == 'integer':
            if 'format' in property_dict:
                if property_dict['format'] == 'int16':
                    return 'int16_t'
                if property_dict['format'] == 'uint16':
                    return 'uint16_t'
                if property_dict['format'] == 'int32':
                    return 'int32_t'
                if property_dict['format'] == 'uint32':
                    return 'uint32_t'
                if property_dict['format'] == 'int64':
                    return 'int64_t'
                if property_dict['format'] == 'uint64':
                    return 'uint64_t'
            return 'int'

        if spec_type == 'boolean':
            return 'bool'

        if spec_type == 'array':
            array_of = get_property_type(property_dict['items'])
            return 'std::vector<{}>'.format(array_of)

        if spec_type == 'object':
            return 'object'

    elif '$ref' in property_dict:
        def_ref = property_dict['$ref'].replace('#/definitions/', '')
        return '{}'.format(caseize_class(def_ref))

    print('**** error: unknown property {} ****'.format(property_dict))
    sys.exit(1)


def iterate_multidimensional(original, my_dict, olist):
    if olist is None:
        olist = []
    for k, v in my_dict.items():
        if (isinstance(v, dict)):
            # print(k+":")
            olist = iterate_multidimensional(original, v, olist)
            continue
        elif (isinstance(v, list)):
            for el in v:
                if (isinstance(el, dict)):
                    olist = iterate_multidimensional(original, el, olist)
        if k == '$ref':
            def_ref = v.replace('#/definitions/', '')
            olist = iterate_multidimensional(original, original[def_ref], olist)
            if def_ref not in olist:
                olist.append(def_ref)
    return olist


def is_required(model_def, property):
    return True
    if 'required' in model_def:
        if property in model_def['required']:
            return True
    return False


def generate_cpp_models(spec_file):
    with open(spec_file) as f:
        spec = json.load(f)

    definitions = spec['definitions']

    ref_order = None
    dep_order = iterate_multidimensional(definitions, definitions, ref_order)

    # create a final write ordering that has anything used as reference first followed by everyting else
    for model, model_def in definitions.items():
        if model not in dep_order:
            dep_order.append(model)

    classes = {}

    for model in dep_order:
        model_def = definitions[model]

        classes[model] = {}
        properties = {}

        if 'type' in model_def and model_def['type'] != 'object':
            # no named members, declare as a typedef instead of class
            classes[model]['__typedef__'] = {
                'type': get_property_type(model_def)
            }

        elif 'properties' in model_def:
            for property, property_def in model_def['properties'].items():
                properties[property] = {
                    'type': get_property_type(property_def),
                    'required': is_required(model_def, property)
                }

        elif 'allOf' in model_def:
            for allOf in model_def['allOf']:
                if 'properties' in allOf:
                    for property, property_def in allOf['properties'].items():
                        properties[property] = {
                            'type': get_property_type(property_def),
                            'required': is_required(allOf, property)
                        }
                elif '$ref' in allOf:
                    def_ref = allOf['$ref'].replace('#/definitions/', '')
                    for property, property_def in definitions[def_ref]['properties'].items():
                        properties[property] = {
                            'type': get_property_type(property_def),
                            'required': is_required(allOf, property)
                        }
        else:
            print("**** unhandled model defintion {}".format(model))
            sys.exit(1)

        classes[model]['properties'] = properties

    print('//')
    print('// autogenerated file.  do not edit directly')
    print('//')
    print('')
    print('namespace amber_models {')
    print('')
    for class_name, class_def in classes.items():

        print('')
        print('class {} {{'.format(caseize_class(class_name)))
        print('public:')

        if '__typedef__' in class_def:
            # write out class that operates like a typedef
            print('    {} value;'.format(class_def['__typedef__']['type']))

            print('')
            print('    friend void to_json(json &j, const {} &r) {{'.format(caseize_class(class_name)))
            print('        j = r.value;')
            print('    };')

            print('')
            print('    friend void from_json(const json &j, {} &r) {{'.format(caseize_class(class_name)))
            print('        r.value = j.get<{}>();'.format(class_def['__typedef__']['type']))
            print('    };')
        else:
            for property, property_def in class_def['properties'].items():
                print('    {} {};'.format(property_def['type'], caseize_property(property)))

            # write out friend functions that format to and from json
            print('')
            print('    friend void to_json(json & j, const {} &r) {{'.format(caseize_class(class_name)))
            for property, property_def in class_def['properties'].items():
                print('        j["{}"] = r.{};'.format(property, caseize_property(property)))
            print('    };')

            print('')
            print('    friend void from_json(const json &j, {} &r) {{'.format(caseize_class(class_name)))
            for property, property_def in class_def['properties'].items():
                print('        if (j.contains("{}") and !j.at("{}").empty()) {{r.{} = j.at("{}").get<{}>();}}'.format(property, property, caseize_property(property), property, property_def['type']))
            print('    };')

        print('')
        print('    AMBER_DUMP()')

        print('};')
    print('} // end namespace')


if __name__ == '__main__':
    generate_cpp_models('swagger.json')
