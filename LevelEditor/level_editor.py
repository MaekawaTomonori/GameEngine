import bpy
import bpy_extras
import math
import gpu
import gpu_extras.batch
import copy
import mathutils
import json

bl_info = {
    "name": "23Level Editor",
    "author": "tmnr",
    "version": (0, 1, 0),
    "blender": (3, 31, 0),
    "category": "Object",
}

def draw_menu_manual(self, context):
    self.layout.operator("wm.url_open_preset", text="manual", icon='HELP')

class MYADDON_OT_stretch_vertex(bpy.types.Operator):
    bl_idname = "myaddon.stretch_vertex"
    bl_label = "Stretch Vertex"
    bl_description = "Stretch the selected vertex to the mouse position"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        bpy.data.objects["Cube"].data.vertices[0].co.x += 1.0 
        print("STRETCH")
        return {'FINISHED'}

class MYADDON_OT_create_ico_sphere(bpy.types.Operator):
    bl_idname = "myaddon.create_ico_sphere"
    bl_label = "Create Ico Sphere"
    bl_description = "Create an Ico Sphere at the cursor location"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        bpy.ops.mesh.primitive_ico_sphere_add()
        print("ICO SPHERE CREATED")
        return {'FINISHED'}


class MYADDON_OT_export_scene(bpy.types.Operator, bpy_extras.io_utils.ExportHelper):
    bl_idname = "myaddon.export_scene"
    bl_label = "Export Scene"
    bl_description = "Export the current scene to a file"
    bl_options = {'REGISTER', 'UNDO'}

    filename_ext = ".json"

    def execute(self, context):
        # Placeholder for export logic
        print("EXPORT SCENE")
        self.export()
        self.report({'INFO'}, "Exported scene successfully")
        return {'FINISHED'}

    def export(self):
        print("Exporting scene to file:", self.filepath)

        json_object_root = dict()
        json_object_root["name"] = "Scene"
        json_object_root["objects"] = list()

        # TODO Output all object in this scene
        for object in bpy.context.scene.objects:
            if(object.parent):
                continue
            self.json_parse(json_object_root["objects"], object, 0)

        # encode
        json_text = json.dumps(json_object_root, ensure_ascii=False, cls=json.JSONEncoder, indent=4)
        print("JSON Text:", json_text)

        # write to file
        with open(self.filepath, 'wt', encoding='utf-8') as file:
            file.write(json_text)

    def write_and_print(self, file, text, level):
        indent = '  '
        for i in range(level):
            indent += "\t"

        print(indent + text)
        file.write(indent + text)
        file.write("\n")


    def parse(self, file, object, level):
        self.write_and_print(file, object.type, level)
        trans, rot, scl = object.matrix_world.decompose()
        rot = rot.to_euler()
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y)
        rot.z = math.degrees(rot.z)
        self.write_and_print(file, "T %f %f %f" % (trans.x, trans.y, trans.z), level)
        self.write_and_print(file, "R %f %f %f" % (rot.x, rot.y, rot.z), level)
        self.write_and_print(file, "S %f %f %f" % (scl.x, scl.y, scl.z), level)

        if "file_name" in object:
            self.write_and_print(file, "N %s" % object["file_name"], level)
        
        if "collider" in object:
            self.write_and_print(file, "C %s" % object["collider"], level)
            temp = "CC %f %f %f"
            temp %= (object["collider_center"][0], object["collider_center"][1], object["collider_center"][2])
            self.write_and_print(file, temp, level)
            temp = "CS %f %f %f"
            temp %= (object["collider_size"][0], object["collider_size"][1], object["collider_size"][2])
            self.write_and_print(file, temp, level)

        self.write_and_print(file, 'END', level)
        self.write_and_print(file, '', level)

        for child in object.children:
            self.parse(file, child, level + 1)

    def json_parse(self, parent, object, level):
        json_object = dict()
        json_object["type"] = object.type
        json_object["name"] = object.name

        if( "file_name" in object):
            json_object["file_name"] = object["file_name"]

        # data packing
        trans, rot, scl = object.matrix_world.decompose()
        rot = rot.to_euler()
        rot.x = math.degrees(rot.x)
        rot.y = math.degrees(rot.y) 
        rot.z = math.degrees(rot.z)

        transform = dict()
        transform["translation"] = (trans.x, trans.y, trans.z)
        transform["rotation"] = (rot.x, rot.y, rot.z)
        transform["scale"] = (scl.x, scl.y, scl.z)
        json_object["transform"] = transform

        if ("file_name" in object):
            json_object["file_name"] = object["file_name"]

        if("collider" in object):
            collider = dict()
            collider["type"] = object["collider"]
            collider["center"] = object["collider_center"].to_list()
            collider["size"] = object["collider_size"].to_list()
            json_object["collider"] = collider
        
        # Register to parent 
        parent.append(json_object)
        
        # Children
        if(len(object.children) > 0):
            json_object["children"] = list()
            for child in object.children:
                self.json_parse(json_object["children"], child, level + 1)

class OBJECT_PT_file_name(bpy.types.Panel):
    """object profile name panel"""
    bl_idname = "OBJECT_PT_file_name"
    bl_label = "File Name"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        if "file_name" in context.object:
            self.layout.prop(context.object, '["file_name"]', text=self.bl_label)
        else:
            self.layout.operator(MYADDON_OT_add_filename.bl_idname, text="Add File Name", icon='ADD')
        self.layout.separator()
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text=MYADDON_OT_export_scene.bl_label, icon='EXPORT')
        self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text=MYADDON_OT_create_ico_sphere.bl_label, icon='MESH_ICOSPHERE')

class MYADDON_OT_add_filename(bpy.types.Operator):
    bl_idname = "myaddon.add_filename"
    bl_label = "Add File Name"
    bl_description = "Add a file name to the object"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        context.object["file_name"] = ""

        return {'FINISHED'}

class TOPBAR_MT_my_menu(bpy.types.Menu):
    bl_idname = "TOPBAR_MT_my_menu"
    bl_label = "MyMenu"
    bl_description = "My custom menu"

    def draw(self, context):
        self.layout.operator(MYADDON_OT_stretch_vertex.bl_idname, text="MYADDON_OT_stretch_vertex", icon='HELP')
        self.layout.operator(MYADDON_OT_create_ico_sphere.bl_idname, text="MYADDON_OT_create_ico_sphere", icon='HELP')
        self.layout.operator(MYADDON_OT_export_scene.bl_idname, text="MYADDON_OT_export_scene", icon='HELP')

    def submenu(self, context):
        self.layout.menu(TOPBAR_MT_my_menu.bl_idname)

class DrawCollider:
    handle = None

    def draw_collider():
        vertices = { "pos" : [] }
        indices = []

        offsets = [
            [-0.5, -0.5, -0.5],
            [0.5, -0.5, -0.5],
            [-0.5, 0.5, -0.5],
            [0.5, 0.5, -0.5],
            [-0.5, -0.5, 0.5],
            [0.5, -0.5, 0.5],
            [-0.5, 0.5, 0.5],
            [0.5, 0.5, 0.5],
        ]

        size = [2.0, 2.0, 2.0]

        for object in bpy.context.scene.objects:
            if not "collider" in object:
                continue

            center = mathutils.Vector((0, 0, 0))
            size = mathutils.Vector((2, 2, 2))

            center[0] = object["collider_center"][0]
            center[1] = object["collider_center"][1]
            center[2] = object["collider_center"][2]
            size[0] = object["collider_size"][0]
            size[1] = object["collider_size"][1]
            size[2] = object["collider_size"][2]

            start = len(vertices["pos"])

            for offset in offsets:
                pos = copy.copy(center)
                pos[0] += offset[0] * size[0]
                pos[1] += offset[1] * size[1]
                pos[2] += offset[2] * size[2]
                pos = object.matrix_world @ pos
                vertices['pos'].append(pos)

            indices.append([start + 0, start + 1])
            indices.append([start + 2, start + 3])
            indices.append([start + 0, start + 2])
            indices.append([start + 1, start + 3])

            indices.append([start + 4, start + 5])
            indices.append([start + 6, start + 7])
            indices.append([start + 4, start + 6])
            indices.append([start + 5, start + 7])

            indices.append([start + 0, start + 4])
            indices.append([start + 1, start + 5])
            indices.append([start + 2, start + 6])
            indices.append([start + 3, start + 7])

        shader = gpu.shader.from_builtin('UNIFORM_COLOR')

        batch = gpu_extras.batch.batch_for_shader(shader, 'LINES', {"pos": vertices["pos"]}, indices=indices)

        color = [0.5, 1.0, 1.0, 1.0]
        shader.bind()
        shader.uniform_float("color", color)

        batch.draw(shader)

class MYADDON_OT_add_collider(bpy.types.Operator):
    bl_idname = "myaddon.add_collider"
    bl_label = "Add Collider"
    bl_description = "Add a collider to the selected object"
    bl_options = {'REGISTER', 'UNDO'}

    def execute(self, context):
        context.object["collider"] = "BOX"
        context.object["collider_center"] = mathutils.Vector((0.0, 0.0, 0.0))
        context.object["collider_size"] = mathutils.Vector((2.0, 2.0, 2.0))

        return {'FINISHED'}

class MYADDON_PT_add_collider(bpy.types.Panel):
    """Panel to add collider to the object"""
    bl_idname = "MYADDON_PT_add_collider"
    bl_label = "Add Collider"
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    bl_context = "object"

    def draw(self, context):
        if "collider" in context.object:
            self.layout.prop(context.object, '["collider"]', text="Collider Type")
            self.layout.prop(context.object, '["collider_center"]', text="Collider Center")
            self.layout.prop(context.object, '["collider_size"]', text="Collider Size")
        else:
            self.layout.operator(MYADDON_OT_add_collider.bl_idname, text="Add Collider", icon='ADD')

classes = (
    MYADDON_OT_stretch_vertex,
    MYADDON_OT_create_ico_sphere,
    MYADDON_OT_export_scene,
    TOPBAR_MT_my_menu,
    OBJECT_PT_file_name,
    MYADDON_OT_add_filename,
    MYADDON_OT_add_collider,
    MYADDON_PT_add_collider,
)

def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_editor_menus.append(TOPBAR_MT_my_menu.submenu)
    DrawCollider.handle = bpy.types.SpaceView3D.draw_handler_add(DrawCollider.draw_collider, (), 'WINDOW', 'POST_VIEW')
    print("Level Editor Add-on Registered")

def unregister():
    bpy.types.TOPBAR_MT_editor_menus.remove(TOPBAR_MT_my_menu.submenu)
    bpy.types.SpaceView3D.draw_handler_remove(DrawCollider.handle, 'WINDOW')
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
    print("Level Editor Add-on Unregistered")



