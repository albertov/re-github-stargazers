module Stargazer = {
    let component = ReasonReact.statelessComponent("Stargazer");
    let make = (~value:Github.stargazer, _children) => {
        ...component,
        render: _self =>
            <div className="stargazer item">
                <img alt="Avatar" className="ui avatar image" src=value.avatarUrl />
                <div className="content">
                    <div className="header">(ReasonReact.string(value.login))</div>
                </div>
            </div>
    };
};

let component = ReasonReact.statelessComponent("Stargazers");

let make = (~stargazers, _children) => {
    ...component,
    render: _self => {
        <div className="ui list stargazers">
            (switch(stargazers) {
            | Some(gazers) => ReasonReact.array(gazers |> Array.map(value=>
                <Stargazer value key=value. Github.login />))
            | None => ReasonReact.null
            })
        </div>
    }
}