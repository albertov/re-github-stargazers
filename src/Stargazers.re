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

let make = (~stargazers, ~maxStargazers=20, _children) => {
    ...component,
    render: _self => {
        let limitedStargazers = {
            let count = Js.Math.min_int(maxStargazers, Array.length(stargazers));
            Array.sub(stargazers, 0, count);
        };
        <div className="ui list stargazers">
            (ReasonReact.array(limitedStargazers |> Array.map(value =>
                <Stargazer value key=value. Github.login />
            )))
        </div>
    }
}