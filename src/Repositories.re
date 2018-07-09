open ReasonReact;

module License = {
    let component = ReasonReact.statelessComponent("License");
    let make = (~license:Github.license, _children) => {
        ...component,
        render: _self =>
            <span className="license" title=license.name>(string(license.spdxId))</span>
    };
};

module StargazersCount = {
    let component = ReasonReact.statelessComponent("StargazersCount");
    let make = (~value, ~onClick=(_=>()), _children) => {
        ...component,
        render: _self =>
            <a href="#" onClick>
                <i className="icon star" />
                <span>(string(string_of_int(value)))</span>
            </a>
    };
};


module Repository = {
    let component = ReasonReact.statelessComponent("Repository");
    let make = (~repository:Github.repository, ~onStargazersClick=(_=>()), children) => {
        ...component,

        render: _self =>
            <div className="item repository">
                <div className="content">
                    <div className="header">
                        <a href=repository.htmlUrl>(string(repository.fullName))</a>
                    </div>
                    <div className="description">
                        <StargazersCount value=repository.stargazerCount onClick=onStargazersClick />
                        (switch (repository.license) {
                        | Some(license) => <License license/>
                        | None          => null
                        })
                    </div>
                    (array(children))
                </div>
            </div>
    };
};

let component = ReasonReact.statelessComponent("Repositories");

let make = (~repositories:array(Github.repository), ~onStargazersClick=(_=>()), _children) => {
    ...component,
    render: _self =>
        <div className="ui list">
            (array(repositories |> Array.map(repository =>
            <Repository
                key=repository. Github.fullName 
                repository
                onStargazersClick=(ev => {
                    ReactEventRe.Mouse.preventDefault(ev);
                    onStargazersClick(repository)
                })
                />
            )))
        </div>
};