
type action =
  | SetText(string)
  | Submit;

type state = string;

let component = ReasonReact.reducerComponent("TextInput");

let make = (~value, ~onChange, _children) => {
    ...component,
    initialState: () => value,
    reducer: action =>
        switch(action) {
        | SetText(value) => (_state => ReasonReact.Update(value))
        | Submit => (state => ReasonReact.SideEffects(_self => onChange(state)))
        },
    render: ({state,send}) => {
        <div className="ui icon input">
            <input
                type_="text"
                placeholder="Enter text..."
                value=state
                onChange=(event => {
                    let value = ReactDOMRe.domElementToObj( ReactEventRe.Form.target(event))##value;
                    send(SetText(value));
                })
                onKeyDown=(
                    event =>
                        if (ReactEventRe.Keyboard.keyCode(event) === 13) {
                            ReactEventRe.Keyboard.preventDefault(event);
                            send(Submit);
                        })
            />
            <i className="circular search link icon"
                onClick=(_event => send(Submit))>
            </i>
        </div>
    }
}