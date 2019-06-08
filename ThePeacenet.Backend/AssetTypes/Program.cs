using Microsoft.Xna.Framework.Content;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Linq.Expressions;
using ThePeacenet.Backend.Data;
using ThePeacenet.Gui.Controls;
using ThePeacenet.Backend.OS;
using System.Reflection;
using ThePeacenet.Gui.Windowing;

namespace ThePeacenet.Backend.AssetTypes
{
    public class Program : Asset
    {
        public Program(ProgramData data, ContentManager content, Func<Window, ContentManager, IUserLand, Control> windowBuilder) : base(data.Name.ToLower().Replace(" ", "_"), content)
        {
            this.Name = data.Name;
            this.Description = data.Description;
            this.LauncherCategory = data.LauncherCategory;
            this.LauncherIcon = data.LauncherIcon;
            this.UnlockedByDefault = data.UnlockedByDefault;
            this.SingleInstance = data.IsSingleInstance;
            this.EnableMinimizeMaximize = data.WindowEnableMinMax;
            this.RamUsage = data.RamUsage;
            this.Gui = data.Gui;
            WindowBuilder = windowBuilder;
        }

        public string Name { get; }
        public string Description { get; }
        public string LauncherCategory { get; }
        public string LauncherIcon { get; }
        public ProgramGui Gui { get; }
        public bool UnlockedByDefault { get; }
        public bool EnableMinimizeMaximize { get; }
        public bool SingleInstance { get; }
        public RamUsage RamUsage { get; }
        public Func<Window, ContentManager, IUserLand, Control> WindowBuilder { get; }
    }

    public class ProgramPage
    {
        public ProgramPage(string id, Func<GuiHandler, ContentManager, IUserLand, Control> builder)
        {
            PageId = id;
            Builder = builder;
        }

        public string PageId { get; }
        public Func<GuiHandler, ContentManager, IUserLand, Control> Builder { get; }
    }

    public class ProgramData : AssetBuilder<Program>
    {
        public string Name { get; set; }
        public string Description { get; set; }
        public string LauncherCategory { get; set; }
        public string LauncherIcon { get; set; }

        [ContentSerializer(Optional = true)]
        public bool UnlockedByDefault { get; set; } = true;

        [ContentSerializer(Optional = true)]
        public bool WindowEnableMinMax { get; set; } = true;

        [ContentSerializer(Optional = true)]
        public bool IsSingleInstance { get; set; } = true;

        [ContentSerializer(CollectionItemName = "Extension", Optional = true)]
        public List<string> SupportedFileExtensions { get; set; } = new List<string>();

        [ContentSerializer(Optional = true)]
        public RamUsage RamUsage { get; set; } = RamUsage.Low;

        public ProgramGui Gui { get; set; }

        public override Program Build(ItemContainer items)
        {
            try
            {
                Console.WriteLine("Compiling {0}...", Name);

                Console.WriteLine(" > Event handler class: {0}", Gui.EventHandlerClass);

                var evcType = FindType(typeof(GuiHandler), Gui.EventHandlerClass);

                var evcVariable = Expression.Variable(evcType, "eventHandlerObject");

                Console.WriteLine(evcVariable.ToString());

                var windowParameter = Expression.Parameter(typeof(Window), "window");
                var contentManagerParameter = Expression.Parameter(typeof(ContentManager), "content");
                var userLandParameter = Expression.Parameter(typeof(IUserLand), "userLand");

                List<Expression> lambdaBody = new List<Expression>
                {
                    Expression.Assign(
                        evcVariable,
                        Expression.New(evcType.GetConstructor(Type.EmptyTypes))
                    ),
                    Expression.Call(
                        windowParameter,
                        typeof(Window).GetMethod("SetGuiHandler", new[] { typeof(GuiHandler) }),
                        evcVariable
                    )
                };

                lambdaBody.Add(Expression.Call(
                        evcVariable,
                        evcType.GetMethod("Initialize", new[] { typeof(Window) }),
                        windowParameter
                    ));


                var winType = typeof(Window);

                Console.WriteLine(" > Compiling window events...");

                foreach (var eventBind in Gui.WindowEvents)
                {
                    var eventInfo = winType.GetEvent(eventBind.Name);
                    var handlerInfo = evcType.GetMethod(eventBind.Handler);

                    Console.WriteLine(" >> {0} -> {1}", eventBind.Name, eventBind.Handler);

                    if (eventInfo == null)
                        throw new ProgramCompileException(string.Format("Unrecognized event {0} on control {1}.", eventBind.Name, winType.FullName));

                    if (handlerInfo == null)
                        throw new ProgramCompileException(string.Format("Unrecognized event handler function {0} for control event {1}.{2}.", eventBind.Handler, winType.FullName, eventInfo.Name));

                    // FIXME: Causes System.Security.VerificationException.
                    /*var eventBindCall = Expression.Call(
                            windowParameter,
                            eventInfo.GetAddMethod(),
                            Expression.New(eventInfo.EventHandlerType.GetConstructor(new[] { typeof(object), typeof(IntPtr) }),
                                    evcVariable,
                                    Expression.Constant(handlerInfo.MethodHandle.GetFunctionPointer(), typeof(IntPtr)
                                )
                        ));*/

                    var eventVariable = Expression.Variable(typeof(EventInfo), "event_" + eventInfo.Name + "_" + Guid.NewGuid().ToString().ToIdentifier());
                    var handlerVariable = Expression.Variable(typeof(MethodInfo), "handler_" + handlerInfo.Name + "_" + Guid.NewGuid().ToString().ToIdentifier());

                    var eventBindCall = Expression.Block(
                            new[] { eventVariable, handlerVariable },
                            Expression.Assign(eventVariable,
                                Expression.Call(
                                    Expression.Call(
                                        windowParameter,
                                        typeof(object).GetMethod("GetType", Type.EmptyTypes)
                                    ),
                                    typeof(Type).GetMethod("GetEvent", new[] { typeof(string) }),
                                    Expression.Constant(eventInfo.Name)
                                )
                            ),
                            Expression.Assign(handlerVariable,
                                Expression.Call(
                                    Expression.Call(
                                        evcVariable,
                                        typeof(object).GetMethod("GetType", Type.EmptyTypes)
                                    ),
                                    typeof(Type).GetMethod("GetMethod", new[] { typeof(string) }),
                                    Expression.Constant(handlerInfo.Name)
                                )
                            ),
                            Expression.Call(eventVariable,
                                typeof(EventInfo).GetMethod("AddEventHandler", new[] { typeof(object), typeof(Delegate) }),
                                windowParameter,
                                Expression.Call(handlerVariable,
                                    typeof(MethodInfo).GetMethod("CreateDelegate", new[] { typeof(Type), typeof(object) }),
                                    Expression.MakeMemberAccess(
                                        eventVariable,
                                        typeof(EventInfo).GetProperty("EventHandlerType")
                                    ),
                                    evcVariable
                                )
                            )
                        );



                    Console.WriteLine(" >>> Bind: {0}", eventBindCall.ToString());
                    lambdaBody.Add(eventBindCall);
                }

                var rootControlType = FindType(typeof(Control), Gui.Content.Type);


                List<ParameterExpression> variables = new List<ParameterExpression>
                {
                    evcVariable
                };

                CompileControls(Gui.Content, variables, lambdaBody, evcVariable, contentManagerParameter, userLandParameter, windowParameter, true);

                var windowBuilderBody = Expression.Block(rootControlType, variables, lambdaBody);
                

                var windowBuilder = Expression.Lambda<Func<Window, ContentManager, IUserLand, Control>>(
                    windowBuilderBody,
                    windowParameter,
                    contentManagerParameter,
                    userLandParameter
                    );

                var windowBuilderDelegate = windowBuilder.Compile();

                return new Program(this, items.Content, windowBuilderDelegate);
            }
            catch (Exception ex)
            {
                Console.WriteLine(" > COMPILE ERROR:");
                Console.WriteLine(ex);
            }
            return new Program(this, items.Content, null);
        }

        private Type FindType(Type expected, string typeName)
        {
            Type type = Type.GetType(typeName);
            if (type != null && expected.IsAssignableFrom(type))
                return type;

            foreach (var assembly in AppDomain.CurrentDomain.GetAssemblies())
            {
                type = assembly.GetTypes().FirstOrDefault(x => x.FullName == typeName);
                if (type != null && expected.IsAssignableFrom(type))
                    return type;
            }

            throw new ProgramCompileException(string.Format("Unrecognized type: {0}.  Type must both exist and inherit from {1}.", typeName, expected.FullName));
        }

        public void CompileControls(ControlElement element, List<ParameterExpression> variables, List<Expression> lambdaBody, Expression evcVariable, Expression contentManager, Expression userLand, Expression window, bool isRoot = false)
        {
            var controlType = FindType(typeof(Control), element.Type);

            var variable = Expression.Variable(controlType, element.Name.ToIdentifier());

            lambdaBody.Add(Expression.Assign(
                    variable,
                    Expression.New(controlType.GetConstructor(Type.EmptyTypes))
                ));

            lambdaBody.Add(Expression.Assign(
                    Expression.MakeMemberAccess(
                        variable,
                        controlType.GetProperty("Name")
                    ),
                    Expression.Constant(element.Name)
                ));

            foreach(var property in element.Properties)
            {
                var propertyInfo = controlType.GetProperty(property.Name);

                if (propertyInfo == null)
                {
                    lambdaBody.Add(Expression.Call(
                            variable,
                            controlType.GetMethod("SetAttachedProperty", new[] { typeof(string), typeof(object) }),
                            Expression.Constant(property.Name),
                            Expression.Constant(property.Value, typeof(object))
                        ));
                }
                else
                {
                    lambdaBody.Add(Expression.Assign(
                            Expression.MakeMemberAccess(
                                variable,
                                propertyInfo
                            ),
                            Expression.Constant(property.Value, propertyInfo.PropertyType)
                        ));
                }
            }

            var buildMethod = controlType.GetMethod("Build", new[] { typeof(ContentManager), typeof(IUserLand) });

            if(buildMethod != null)
            {
                lambdaBody.Add(Expression.Call(
                        variable,
                        buildMethod,
                        contentManager,
                        userLand
                    ));
            }

            if(element.Children.Count > 0)
            {
                if(controlType.GetProperty("Items") != null)
                {
                    // iterate through all child elements.
                    foreach(var child in element.Children)
                    {
                        // compile the child control data.
                        CompileControls(child, variables, lambdaBody, evcVariable, contentManager, userLand, window);

                        // Add the child control to its parent.
                        lambdaBody.Add(Expression.Call(
                                Expression.MakeMemberAccess(
                                    variable,
                                    controlType.GetProperty("Items")
                                ),
                                controlType.GetProperty("Items").PropertyType.GetMethod("Add"),
                                variables.Last()
                            ));
                    }
                }
                else if(controlType.GetProperty("Content") != null)
                {
                    if(element.Children.Count > 1)
                    {
                        throw new ProgramCompileException(string.Format("{0} does not support multiple children.", controlType.FullName));
                    }

                    // compile the child control data.
                    CompileControls(element.Children.First(), variables, lambdaBody, evcVariable, contentManager, userLand, window);

                    // Assign this new child as the current control's content.
                    lambdaBody.Add(Expression.Assign(
                            Expression.MakeMemberAccess(
                                variable,
                                controlType.GetProperty("Content")
                            ),
                            variables.Last()
                        ));
                }
                else
                {
                    throw new ProgramCompileException(string.Format("{0} does not support children.", controlType.FullName));
                }
            }

            variables.Add(variable);

            if (isRoot)
            {
                LabelTarget label = Expression.Label(controlType);

                lambdaBody.Add(variable);
            }
        }
    }

    public class ProgramCompileException : Exception
    {
        public ProgramCompileException(string message) : base(message)
        {

        }
    }

    public static class StringExtensions
    {
        public static string ToIdentifier(this string input)
        {
            string output = "";
            
            for(int i = 0; i < input.Length; i++)
            {
                char c = input[i];

                if (!char.IsLetterOrDigit(c) && c != '_')
                    continue;

                if (char.IsDigit(c) && output.Length == 0)
                    output += "_";

                output += c;
            }

            return output;
        }
    }

}
