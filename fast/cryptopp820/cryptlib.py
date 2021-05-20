import xml.etree.ElementTree as ET
from io import StringIO

ns = '{http://schemas.microsoft.com/developer/msbuild/2003}'
xml_name='cryptlib.vcxproj.filters'
if __name__ == '__main__':
    # with open('cryptlib.vcxproj.filters') as proj:
    it = ET.iterparse(xml_name)

    for _,el in it:
        prefix, has_ns, postfix = el.tag.partition('}')
        if has_ns:
            el.tag = postfix
    root = it.root

    items = []

    for g in root:
        for c in g:
            if c.tag == 'ClCompile':
                items.append(c.attrib['Include'])
            elif c.tag == 'ClInclude':
                items.append(c.attrib['Include'])
            else:
                break
    items.sort()
    print(items)
    with open('BUILD.gn.tmp','w') as f:
        for i in items:
            f.write('"' + i+ '",''\n')